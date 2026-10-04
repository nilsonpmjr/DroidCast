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

## Part 2: navigation and engine lifecycle

- Added a category picker for the implemented Video, Audio, Device, Window and
  Recording settings. Category and text filters combine; connection and input
  destinations are directly reachable. This is not yet all 16 planned routes.
- Added an opt-in, versioned bridge to the bundled C fork. The desktop distinguishes
  Starting, Streaming (first frame), Stopping, Ended, Failed and Disconnected.
- Stop sends a quit request through the bridge instead of terminating the process.
  Recorder finalization is reported independently, and output-close errors now
  contribute to recorder failure. Timeout kills retain a warning rather than
  being misreported as successful stops.
- Disabled repeated stops and phone mutations during cleanup. A new session can
  start only after the old process exits.
- Added desktop lifecycle/failure tests and a real pipe-to-SDL bridge test. These
  passed on Linux; playable recording and disconnect checks with hardware remain
  required before closing tickets 1–3.

See `desktop/BRIDGE.md` for the protocol and limitations. The phone toolbar still
uses ADB; this bridge currently carries lifecycle events and quit only.

## Part 3: input modes and recording preferences

- Keyboard and mouse now support Disabled as well as SDK and UHID modes.
- Added exclusive SDK key-injection choices (default, prefer text, raw events),
  key-repeat suppression, mouse-hover suppression and UHID gamepad forwarding.
- SDK-only controls become inactive for other input modes, and read-only disables
  advanced input options. Saved inactive choices are not passed to the engine.
  AOA/OTG remains outside this slice because its platform and transport rules
  need their own implementation.
- Added MKV/MP4 recording container selection with matching generated filenames,
  independent recording rotation, and a time limit applying to the whole session.
  Container and rotation arguments are emitted only for recording launches.
- Reject MP4 plus raw audio before starting, with actionable feedback. Audio-only
  containers and no-playback recording modes remain planned.
- Added tests for mode compatibility, argument isolation, disabled-mode
  persistence, recording validation and UI enablement. Hardware gamepad and
  recording-format checks remain required.

## Part 4: live mirror-window controls

- Added fullscreen, fit, pixel-perfect size, left/right rotation and separate
  pause/resume controls, dispatched through the private bridge onto scrcpy's SDL
  thread. No global key injection is involved.
- Availability requires a first frame and an explicit engine capability event.
  Host-window controls remain available in read-only sessions; Android-mutating
  controls remain disabled. A paused image does not pause audio or recording.
- Commands are serialized, acknowledged and time-limited. An unconfirmed command
  disables further window requests until restart rather than retrying a toggle.
  Stop is independent. Resize in fullscreen/maximized mode is rejected visibly.
- Kept the existing palette and used labelled native buttons, grouped paired
  actions, and local feedback with explicit disabled states per UI/UX guidance.
- Verification includes desktop controller tests, UI tests at 920×680, and a
  pipe/SDL test verifying all seven command bytes and unknown-byte handling.
  Visual review used the offscreen native application. Physical-device mirroring,
  window-manager behavior, Windows and macOS still require manual validation.

This advances ticket 16. Reset-video and other live Android actions remain planned.

## Part 5: advanced capture settings and device inspection

- Added capture crop, capture orientation (including locking/flipping), display
  ID, explicit video encoder and an option to disable automatic downsizing.
- Optional crop and encoder fields have validation beside the field. Invalid
  values block session launch at both UI and engine boundaries; the UI takes the
  user to the offending field. Preferences remain available for correction.
- Added a selected-device inspection workflow using the bundled engine's
  `--list-displays --list-encoders`. Reports show their target serial and remain
  plain text. This slice supports manual copying of values, not automatic
  capability parsing or a promise that a codec/encoder combination will work.
- Inspection does not start mirroring. It is mutually exclusive with mirror
  launch, has cancellation, a 20-second timeout, bounded output, and failure
  feedback. It does not inherit the desktop session token. Inspection is allowed
  in read-only mode; listing still uses scrcpy's normal ADB/server workflow.
- Tests cover persistence/arguments, malformed values, target isolation, canceled
  and failed inspections, excessive output, timeout recovery, and UI validation.
  Real-phone resource enumeration and successful custom capture configurations
  still require hardware verification.

This advances ticket 17. Camera, virtual display, encoder-specific codec options
and automatic capability-driven selectors remain future slices.
