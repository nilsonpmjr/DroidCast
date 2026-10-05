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
static bool camera_controls_reported;
static bool android_controls_reported;
static bool clipboard_controls_reported;
static bool clipboard_copy_pending;
static bool fps_controls_reported;

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
    for (int i = 0; i < count; ++i) {
        if (bytes[i] && strchr("FWZLRPUTt+-01NSCDVYyIi", bytes[i])) {
            SDL_Event event = {
                .user = {.type = SC_EVENT_DESKTOP_WINDOW_COMMAND,
                         .code = bytes[i]},
            };
            SDL_PushEvent(&event);
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
    clipboard_controls_reported = false;
    clipboard_copy_pending = false;
    fps_controls_reported = false;
    input_timer = SDL_AddTimer(50, read_commands, NULL);
    sc_desktop_bridge_report(input_timer ? "bridge-ready" : "bridge-error");
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
}
