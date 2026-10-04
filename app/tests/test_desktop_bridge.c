#include <assert.h>
#include <stdlib.h>
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
    assert(SDL_WaitEventTimeout(&event, 1000));
    assert(event.type == SDL_EVENT_QUIT);
    sc_desktop_bridge_stop();

    sc_desktop_bridge_start();
    assert(write(pipes[1], "?FWZLRPUTt+-01NSCDV", 19) == 19);
    const char *expected = "FWZLRPUTt+-01NSCDV";
    for (int i = 0; i < 18; ++i) {
        assert(SDL_WaitEventTimeout(&event, 1000));
        assert(event.type == SC_EVENT_DESKTOP_WINDOW_COMMAND);
        assert(event.user.code == expected[i]);
    }
    assert(!SDL_WaitEventTimeout(&event, 100));
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
