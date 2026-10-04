#include "desktop_bridge.h"

#include <stdbool.h>
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
    if (!count || (count > 0 && memchr(bytes, 'Q', (size_t) count))) {
        sc_push_event(SDL_EVENT_QUIT);
        return 0;
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
    input_timer = SDL_AddTimer(50, read_commands, NULL);
    sc_desktop_bridge_report(input_timer ? "bridge-ready" : "bridge-error");
}

void
sc_desktop_bridge_first_frame(void) {
    // Called exclusively from the SDL thread, after the texture is uploaded.
    if (!frame_reported) {
        frame_reported = true;
        sc_desktop_bridge_report("first-frame");
    }
}

void
sc_desktop_bridge_stop(void) {
    if (input_timer) {
        SDL_RemoveTimer(input_timer);
        input_timer = 0;
    }
}
