# Private desktop bridge, version 1

The bundled fork opts into the bridge when `DROIDCAST_SESSION_TOKEN` contains
exactly 32 lowercase hexadecimal characters. Regular scrcpy invocations retain
their normal stdin behavior. The desktop generates a new token per launch.

- Parent → child stdin: the byte `Q` requests an SDL quit event. EOF does the same.
  This follows scrcpy's normal window-close cleanup, including joining the recorder.
- After the matching readiness event, the parent can send window or camera
  commands below. They are whitelisted bytes delivered to the SDL thread, not
  shell commands or synthesized keyboard shortcuts. Unknown bytes are ignored;
  `Q` takes priority.
- Child → parent stdout: `DROIDCAST/1 <token> <event>\n`.
- Events: `bridge-ready`, `bridge-error`, `first-frame`, `recording-finalized`,
  `recording-error`, `ended`, `disconnected`, `failed`.
- Window events: `window-controls-ready` and
  `window-result:<byte>:handled` / `window-result:<byte>:unavailable`.
- Camera events: `camera-controls-ready` and
  `camera-result:<byte>:handled` / `camera-result:<byte>:unavailable`.
- `first-frame` is emitted once, after uploading and rendering a video frame.
  Starting a process alone is not evidence of video readiness.
- Recording status comes from the recorder callback after trailer/output close;
  process completion is still required before a new session can begin.
- Stderr remains ordinary diagnostic output. The desktop accepts only exact
  protocol lines matching the current session token, and bounds partial lines.

| Byte | Computer-window action |
| --- | --- |
| `F` | Toggle fullscreen |
| `W` | Resize to fit |
| `Z` | Resize to pixel-perfect size |
| `L` / `R` | Rotate the displayed image left / right |
| `P` / `U` | Pause / resume the displayed image |

| Byte | Live camera action |
| --- | --- |
| `T` / `t` | Request torch on / off |
| `+` / `-` | Request relative zoom in / out |

Camera commands are advertised only for a camera stream and require the Android
control channel. A handled result confirms that the request entered that channel;
the camera HAL may still reject unsupported torch or zoom behavior. Zoom is
unavailable while the mirror image is paused. DroidCast therefore reports a
request, not an invented torch state or zoom value.

Ordinary window commands do not mutate Android and are allowed in read-only mode.
With `--flex-display`, resizing propagates to the Android virtual display;
DroidCast disables that mode when read-only is selected. Pause
does not pause the phone, audio, or recording. Resize is rejected in fullscreen
or maximized mode. A handled response acknowledges dispatch, not a guarantee that
the window manager honored the requested geometry/fullscreen state. The desktop
does not maintain optimistic fullscreen or paused toggle state, so keyboard
shortcuts in the mirror cannot leave a stale toggle in the desktop UI.

Only one bridge command can be outstanding. After two seconds without a matching
response, the desktop disables that command family until a new session starts.
It never retries toggles automatically, and late replies cannot complete a later
command in the same session. A camera timeout does not disable window controls,
and Stop remains available independently.

The desktop gives normal cleanup five seconds, then kills an unresponsive child
and preserves an incomplete-recording warning. Killing the parent or a machine
power loss still cannot guarantee a playable recording. This protocol requires
the locally built fork (`DROIDCAST_BUILD_FORK=ON`); upstream release binaries do
not implement it and cannot provide these lifecycle guarantees.

Verification includes fake-process desktop tests and `app/tests/test_desktop_bridge.c`,
a Unix pipe/SDL integration test registered in debug Meson builds. The Windows
pipe implementation is included but still requires Windows validation.
