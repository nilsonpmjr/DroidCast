#include "desktop_bridge.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>

#ifdef _WIN32
# include <windows.h>
#else
# include <poll.h>
# include <unistd.h>
#endif

#include "events.h"

static const char *session_token;
static SDL_TimerID input_timer;
static bool frame_reported;
static bool camera_controls_reported;
static bool android_controls_reported;
static bool app_controls_reported;
static bool clipboard_controls_reported;
static bool clipboard_copy_pending;
static bool fps_controls_reported;

enum sc_desktop_input_state {
    SC_DESKTOP_INPUT_COMMAND,
    SC_DESKTOP_INPUT_APP_LENGTH,
    SC_DESKTOP_INPUT_APP_PAYLOAD,
};

static enum sc_desktop_input_state input_state;
static uint8_t app_payload_length;
static uint8_t app_payload_received;
static char app_payload[256];

static bool
is_ascii_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool
is_ascii_alnum(char c) {
    return is_ascii_alpha(c) || (c >= '0' && c <= '9');
}

static bool
is_exact_package(const char *package, size_t length) {
    bool segment_start = true;
    for (size_t i = 0; i < length; ++i) {
        char c = package[i];
        if (segment_start) {
            if (!is_ascii_alpha(c)) {
                return false;
            }
            segment_start = false;
        } else if (c == '.') {
            segment_start = true;
        } else if (!is_ascii_alnum(c) && c != '_') {
            return false;
        }
    }
    return length && !segment_start;
}

static void
report_app_unavailable(void) {
    sc_desktop_bridge_report("app-result:A:unavailable");
}

static void
push_app_command(void) {
    app_payload[app_payload_length] = '\0';
    if (!is_exact_package(app_payload, app_payload_length)) {
        report_app_unavailable();
        return;
    }

    char *package = strdup(app_payload);
    if (!package) {
        report_app_unavailable();
        return;
    }
    SDL_Event event = {
        .user = {
            .type = SC_EVENT_DESKTOP_WINDOW_COMMAND,
            .code = 'A',
            .data1 = package,
        },
    };
    if (!SDL_PushEvent(&event)) {
        free(package);
        report_app_unavailable();
    }
}

// Returns false when the quit command has stopped the input timer.
static bool
process_input_byte(uint8_t byte) {
    switch (input_state) {
        case SC_DESKTOP_INPUT_APP_LENGTH:
            if (!byte) {
                input_state = SC_DESKTOP_INPUT_COMMAND;
                report_app_unavailable();
            } else {
                app_payload_length = byte;
                app_payload_received = 0;
                input_state = SC_DESKTOP_INPUT_APP_PAYLOAD;
            }
            return true;
        case SC_DESKTOP_INPUT_APP_PAYLOAD:
            app_payload[app_payload_received++] = (char) byte;
            if (app_payload_received == app_payload_length) {
                input_state = SC_DESKTOP_INPUT_COMMAND;
                push_app_command();
            }
            return true;
        case SC_DESKTOP_INPUT_COMMAND:
            break;
    }

    if (byte == 'Q') {
        sc_push_event(SDL_EVENT_QUIT);
        return false;
    }
    if (byte == 'A') {
        input_state = SC_DESKTOP_INPUT_APP_LENGTH;
        return true;
    }
    if (byte && strchr("FWZLRPUTt+-01NSCDVYyIi", byte)) {
        SDL_Event event = {
            .user = {.type = SC_EVENT_DESKTOP_WINDOW_COMMAND, .code = byte},
        };
        SDL_PushEvent(&event);
    }
    return true;
}

// Only this timer reads stdin. Bounded non-blocking reads also make shutdown
// safe when the desktop disappears without sending the quit byte.
static Uint32 SDLCALL
read_commands(void *userdata, SDL_TimerID timer_id, Uint32 interval) {
    (void) userdata;
    (void) timer_id;
    char bytes[64];
    int count;
#ifdef _WIN32
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD available, received;
    if (!PeekNamedPipe(input, NULL, 0, NULL, &available, NULL)) {
        sc_push_event(SDL_EVENT_QUIT);
        return 0;
    }
    if (!available) {
        return interval;
    }
    DWORD size = available < sizeof(bytes) ? available : sizeof(bytes);
    if (!ReadFile(input, bytes, size, &received, NULL)) {
        sc_push_event(SDL_EVENT_QUIT);
        return 0;
    }
    count = (int) received;
#else
    struct pollfd input = {.fd = STDIN_FILENO, .events = POLLIN};
    if (poll(&input, 1, 0) <= 0) {
        return interval;
    }
    count = (int) read(STDIN_FILENO, bytes, sizeof(bytes));
#endif
    if (count <= 0) {
        sc_push_event(SDL_EVENT_QUIT);
        return 0;
    }
    for (int i = 0; i < count; ++i) {
        if (!process_input_byte((uint8_t) bytes[i])) {
            return 0;
        }
    }
    return interval;
}

void
sc_desktop_bridge_report(const char *event) {
    if (session_token) {
        fprintf(stdout, "DROIDCAST/1 %s %s\n", session_token, event);
        fflush(stdout);
    }
}

void
sc_desktop_bridge_start(void) {
    const char *token = getenv("DROIDCAST_SESSION_TOKEN");
    if (!token || strlen(token) != 32
            || strspn(token, "0123456789abcdef") != 32) {
        return;
    }
    session_token = token;
    frame_reported = false;
    camera_controls_reported = false;
    android_controls_reported = false;
    app_controls_reported = false;
    clipboard_controls_reported = false;
    clipboard_copy_pending = false;
    fps_controls_reported = false;
    input_state = SC_DESKTOP_INPUT_COMMAND;
    app_payload_length = 0;
    app_payload_received = 0;
    input_timer = SDL_AddTimer(50, read_commands, NULL);
    sc_desktop_bridge_report(input_timer ? "bridge-ready" : "bridge-error");
}

void
sc_desktop_bridge_app_controls_ready(void) {
    if (!app_controls_reported) {
        app_controls_reported = true;
        sc_desktop_bridge_report("app-controls-ready");
    }
}

void
sc_desktop_bridge_fps_controls_ready(void) {
    if (!fps_controls_reported) {
        fps_controls_reported = true;
        sc_desktop_bridge_report("fps-controls-ready");
    }
}

void
sc_desktop_bridge_fps_state(bool started) {
    sc_desktop_bridge_report(started ? "fps-state:started"
                                     : "fps-state:stopped");
}

void
sc_desktop_bridge_fps_sample(unsigned rendered, unsigned skipped) {
    char event[64];
    snprintf(event, sizeof(event), "fps-sample:%u:%u", rendered, skipped);
    sc_desktop_bridge_report(event);
}

void
sc_desktop_bridge_clipboard_controls_ready(void) {
    if (!clipboard_controls_reported) {
        clipboard_controls_reported = true;
        sc_desktop_bridge_report("clipboard-controls-ready");
    }
}

void
sc_desktop_bridge_clipboard_copy_pending(void) {
    clipboard_copy_pending = true;
}

void
sc_desktop_bridge_clipboard_copy_complete(bool success) {
    if (clipboard_copy_pending) {
        clipboard_copy_pending = false;
        sc_desktop_bridge_report(success ? "clipboard-result:Y:handled"
                                         : "clipboard-result:Y:unavailable");
    }
}

void
sc_desktop_bridge_android_controls_ready(void) {
    if (!android_controls_reported) {
        android_controls_reported = true;
        sc_desktop_bridge_report("android-controls-ready");
    }
}

void
sc_desktop_bridge_camera_controls_ready(void) {
    if (!camera_controls_reported) {
        camera_controls_reported = true;
        sc_desktop_bridge_report("camera-controls-ready");
    }
}

void
sc_desktop_bridge_first_frame(void) {
    // Called exclusively from the SDL thread, after the texture is uploaded.
    if (!frame_reported) {
        frame_reported = true;
        sc_desktop_bridge_report("first-frame");
        sc_desktop_bridge_report("window-controls-ready");
    }
}

void
sc_desktop_bridge_stop(void) {
    if (input_timer) {
        SDL_RemoveTimer(input_timer);
        input_timer = 0;
    }
    clipboard_copy_pending = false;
    input_state = SC_DESKTOP_INPUT_COMMAND;
}
