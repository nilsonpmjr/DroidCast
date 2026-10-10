#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <SDL3/SDL.h>

#include "desktop_bridge.h"
#include "events.h"

// Exercise the bridge's real timer, pipe reads and SDL delivery without a phone.
bool
sc_push_event_impl(uint32_t type, void *ptr, const char *name) {
    (void) ptr;
    (void) name;
    SDL_Event event = {.type = type};
    return SDL_PushEvent(&event);
}

int main(void) {
    assert(SDL_Init(SDL_INIT_EVENTS));
    int pipes[2];
    assert(!pipe(pipes));
    assert(dup2(pipes[0], STDIN_FILENO) == STDIN_FILENO);
    close(pipes[0]);
    assert(!setenv("DROIDCAST_SESSION_TOKEN", "invalid", 1));
    sc_desktop_bridge_start();
    assert(write(pipes[1], "Q", 1) == 1);
    SDL_Event event;
    assert(!SDL_WaitEventTimeout(&event, 100));

    assert(!setenv("DROIDCAST_SESSION_TOKEN", "0123456789abcdef0123456789abcdef", 1));
    sc_desktop_bridge_start();
    sc_desktop_bridge_first_frame();
    sc_desktop_bridge_first_frame();
    sc_desktop_bridge_first_audio_packet();
    sc_desktop_bridge_first_audio_packet();
    assert(SDL_WaitEventTimeout(&event, 1000));
    assert(event.type == SDL_EVENT_QUIT);
    sc_desktop_bridge_stop();

    sc_desktop_bridge_start();
    assert(write(pipes[1], "?FWZLRPUTt+-01NSCDVYyIi", 23) == 23);
    const char *expected = "FWZLRPUTt+-01NSCDVYyIi";
    for (int i = 0; i < 22; ++i) {
        assert(SDL_WaitEventTimeout(&event, 1000));
        assert(event.type == SC_EVENT_DESKTOP_WINDOW_COMMAND);
        assert(event.user.code == expected[i]);
    }
    assert(!SDL_WaitEventTimeout(&event, 100));
    sc_desktop_bridge_stop();

    // Framed app payloads may span reads, and command bytes inside a package
    // (notably Q) must remain data rather than terminating the child.
    sc_desktop_bridge_start();
    const char *package = "com.Qtest.App";
    uint8_t app_prefix[] = {'A', (uint8_t) strlen(package), 'c', 'o', 'm'};
    assert(write(pipes[1], app_prefix, sizeof(app_prefix)) ==
           (ssize_t) sizeof(app_prefix));
    SDL_Delay(75);
    assert(write(pipes[1], package + 3, strlen(package) - 3) ==
           (ssize_t) strlen(package) - 3);
    assert(SDL_WaitEventTimeout(&event, 1000));
    assert(event.type == SC_EVENT_DESKTOP_WINDOW_COMMAND);
    assert(event.user.code == 'A');
    assert(!strcmp(event.user.data1, package));
    free(event.user.data1);

    const char *invalid_package = "com.bad;rm";
    uint8_t invalid_frame[2 + 10] = {'A', 10};
    memcpy(&invalid_frame[2], invalid_package, 10);
    assert(write(pipes[1], invalid_frame, sizeof(invalid_frame)) ==
           (ssize_t) sizeof(invalid_frame));
    assert(!SDL_WaitEventTimeout(&event, 150));
    sc_desktop_bridge_stop();

    // Closing the parent pipe must also leave through the normal quit route.
    sc_desktop_bridge_start();
    close(pipes[1]);
    assert(SDL_WaitEventTimeout(&event, 1000));
    assert(event.type == SDL_EVENT_QUIT);
    sc_desktop_bridge_stop();
    SDL_Quit();
    return 0;
}
