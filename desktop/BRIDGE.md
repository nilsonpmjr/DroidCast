# Private desktop bridge, version 1

The bundled fork opts into the bridge when `DROIDCAST_SESSION_TOKEN` contains
exactly 32 lowercase hexadecimal characters. Regular scrcpy invocations retain
their normal stdin behavior. The desktop generates a new token per launch.

- Parent → child stdin: the byte `Q` requests an SDL quit event. EOF does the same.
  This follows scrcpy's normal window-close cleanup, including joining the recorder.
- Child → parent stdout: `DROIDCAST/1 <token> <event>\n`.
- Events: `bridge-ready`, `bridge-error`, `first-frame`, `recording-finalized`,
  `recording-error`, `ended`, `disconnected`, `failed`.
- `first-frame` is emitted once, after uploading and rendering a video frame.
  Starting a process alone is not evidence of video readiness.
- Recording status comes from the recorder callback after trailer/output close;
  process completion is still required before a new session can begin.
- Stderr remains ordinary diagnostic output. The desktop accepts only exact
  protocol lines matching the current session token, and bounds partial lines.

The desktop gives normal cleanup five seconds, then kills an unresponsive child
and preserves an incomplete-recording warning. Killing the parent or a machine
power loss still cannot guarantee a playable recording. This protocol requires
the locally built fork (`DROIDCAST_BUILD_FORK=ON`); upstream release binaries do
not implement it and cannot provide these lifecycle guarantees.

Verification includes fake-process desktop tests and `app/tests/test_desktop_bridge.c`,
a Unix pipe/SDL integration test registered in debug Meson builds. The Windows
pipe implementation is included but still requires Windows validation.
