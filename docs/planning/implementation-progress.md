# Incremental implementation

## Part 1: configuration and phone controls

- Preserved the approved native design and palette.
- Added a typed option catalog shared by persistence, validation, launch argument
  generation and settings widgets. Unknown options are never passed through.
- Added searchable settings groups (including matching CLI flag names).
- Added 13 settings: video buffer, display orientation, audio codec/source/buffers,
  read-only mode, touch indicators, screen-off on close, clipboard autosync opt-out,
  fullscreen, borderless window and computer screensaver inhibition.
- Added Back, Home, Recent apps, Power and volume buttons. This initial transport
  is serial-targeted ADB key events, not host keyboard automation or scrcpy IPC.
- Device operations remain serialized and time-limited. Controls target the active
  session when one exists and the selected authorized device otherwise.
- Read-only mode blocks phone controls and installation; a running session retains
  its launch-time policy. Conflicting launch-only device behaviors are suppressed.
  Screenshots remain available.

This is a partial slice of tickets 6, 13, 14, 17 and 18, not completion of them.
All 16 configuration destinations, capability discovery, embedded display,
scrcpy control-protocol integration, live window controls and lifecycle/recording
shutdown improvements remain planned. Settings apply to the next session.

Verification: CMake build and offscreen Qt tests, including settings round-trip,
argument validation, search filtering, command targeting and read-only snapshot.
Hardware validation is still required; no phone interaction is claimed from the
fake-process tests. Windows and macOS are not verified by this Linux build.
