#include <assert.h>
#include <string.h>

#include "demuxer.h"
#include "util/binary.h"

// Feed the real demuxer a bounded in-memory stream without a phone or socket.
static uint8_t stream[64];
static size_t stream_size;
static size_t stream_position;
static unsigned first_packets;
static unsigned media_packets;
static bool ended;
static enum sc_sink_result media_result;

ssize_t
net_recv_all(sc_socket socket, void *buffer, size_t size) {
    (void) socket;
    size_t remaining = stream_size - stream_position;
    size_t count = MIN(size, remaining);
    memcpy(buffer, stream + stream_position, count);
    stream_position += count;
    return count;
}

static bool
sink_open(struct sc_packet_sink *sink, const AVCodecParameters *params,
          const struct sc_stream_session *session) {
    (void) sink;
    assert(params->codec_id == AV_CODEC_ID_OPUS);
    assert(!session);
    return true;
}

static void
sink_close(struct sc_packet_sink *sink) {
    (void) sink;
}

static enum sc_sink_result
sink_push(struct sc_packet_sink *sink, const AVPacket *packet) {
    (void) sink;
    if (packet->pts == AV_NOPTS_VALUE) {
        assert(!first_packets);
        return SC_SINK_OK;
    }
    ++media_packets;
    return media_result;
}

static void
on_first_packet(struct sc_demuxer *demuxer, void *userdata) {
    (void) demuxer;
    (void) userdata;
    assert(media_packets == 1);
    assert(media_result == SC_SINK_OK);
    ++first_packets;
}

static void
on_ended(struct sc_demuxer *demuxer, enum sc_demuxer_status status,
         void *userdata) {
    (void) demuxer;
    (void) status;
    (void) userdata;
    ended = true;
}

static void
append_packet(uint64_t pts) {
    sc_write64be(stream + stream_size, pts);
    sc_write32be(stream + stream_size + 8, 1);
    stream[stream_size + 12] = 0;
    stream_size += 13;
}

static void
check_first_packet(enum sc_sink_result result, unsigned media_count) {
    first_packets = media_packets = 0;
    ended = false;
    media_result = result;
    stream_position = 0;
    stream_size = 4;
    sc_write32be(stream, 0x6f707573); // opus
    append_packet(UINT64_C(1) << 62); // configuration is not readiness
    for (unsigned i = 0; i < media_count; ++i) {
        append_packet(i + 1);
    }

    static const struct sc_packet_sink_ops ops = {
        .open = sink_open,
        .close = sink_close,
        .push = sink_push,
    };
    struct sc_packet_sink sink = {.ops = &ops};
    static const struct sc_demuxer_callbacks cbs = {
        .on_ended = on_ended,
        .on_first_packet = on_first_packet,
    };
    struct sc_demuxer demuxer;
    sc_demuxer_init(&demuxer, "audio-test", (sc_socket) 1, false, &cbs, NULL);
    sc_packet_source_add_sink(&demuxer.packet_source, &sink);
    assert(sc_demuxer_start(&demuxer));
    sc_demuxer_join(&demuxer);
    assert(ended);
    assert(first_packets == (result == SC_SINK_OK && media_count ? 1u : 0u));
}

int
main(void) {
    check_first_packet(SC_SINK_OK, 0);
    check_first_packet(SC_SINK_OK, 2);
    check_first_packet(SC_SINK_KO, 2);
    check_first_packet(SC_SINK_STOPPED, 2);
    return 0;
}
