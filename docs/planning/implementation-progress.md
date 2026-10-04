# Incremental implementation

For a concise view of what works, what remains and the concrete next steps, start
with [the gap register](gap-register.md). This file retains the chronological
implementation record.

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
and automatic capability-driven selectors were future slices at this checkpoint.

## Part 6: virtual-display sessions

- Added a Virtual display settings category: explicit creation toggle, optional
  resolution/density, flexible resizing, exact-package launch, on-screen keyboard
  placement, system decorations, and move-apps-to-primary behavior on close.
- Empty size uses engine defaults; density-only and resolution/density forms are
  validated. No virtual display is created just by changing preferences.
- Inactive values are retained but omitted from launch arguments. Read-only mode
  disables flexible resizing and app launch; it does not suppress an explicitly
  requested virtual display. Exact package names exclude search/force-stop modes.
- Centralized launch validation used by both UI and engine. Conflicting physical
  display IDs and flexible-display/crop combinations fail with actionable feedback.
- Added launch-time alternate-display guards. Existing ADB phone buttons and
  screenshots cannot target the active secondary/virtual session's primary screen
  accidentally; their UI controls are disabled as well. Mirror input and recording
  remain available. Changing next-session settings cannot weaken the active guard.
- Kept the approved palette, clarified dependent controls and removed duplicated
  toggle labels from advanced settings. Added a minimum-window-size layout test.
- Verification covers arguments, invalid input, persistence, inactive settings,
  read-only compatibility, targeting guards and UI dependencies. Full desktop tests
  pass on Linux. Android 10+ virtual-display creation, app behavior and recording
  on hardware still need verification; this does not close ticket 20's release gate.

## Part 7: camera-capture sessions

- Added a Camera settings category and explicit display/camera source selection.
  Android 12+ camera sessions support exact ID or facing, exact size or aspect
  ratio, frame rate, high-speed mode, startup torch and initial zoom.
- Added progressive dependencies for mutually exclusive settings. Exact IDs
  suppress facing; exact sizes suppress the general resolution limit and aspect
  ratio; display-only, virtual-display and advanced input settings become inactive
  in camera mode. Saved inactive choices remain available for later display sessions.
- Changed audio source to an explicit Auto default, matching scrcpy: display
  capture defaults to device output and camera capture defaults to the microphone.
  Choosing output/playback/mic still produces an explicit override.
- Extended idle device inspection with `--list-camera-sizes`. The same bounded,
  target-specific report now contains displays, encoders, cameras, declared sizes
  and rates. The UI warns that Android camera declarations may be inaccurate.
- Added format and range validation for camera IDs, sizes, aspect ratios and zoom.
  High-speed capture without an explicit frame rate is rejected before launch.
- Snapshot camera mode at launch. ADB screenshots and phone-toolbar key events are
  disabled for the active camera session instead of silently targeting the phone
  display; recording and host mirror-window controls remain available.
- Verification covers emitted and suppressed arguments, invalid/inactive values,
  dependent widget states, combined inspection output and launch-time targeting
  guards. Linux automated tests do not prove camera availability or supported
  size/rate combinations; Android 12+ hardware testing remains required.

This advances ticket 21. Live torch/zoom controls were deferred to the next
slice; automatic structured selectors and hardware testing remained open.

## Part 8: live camera controls

- Extended the opt-in private bridge with explicit camera capability and result
  events. Whitelisted bytes reach scrcpy's SDL thread and then its existing Android
  control-message queue; no global shortcuts or shell commands are synthesized.
- Added contextual Torch on/off and Zoom out/in controls to the active-session
  workspace. The group appears only for a running camera session, remains disabled
  until first-frame capability readiness, and is inaccessible in read-only mode.
- Results deliberately say that a request was sent. Queue acceptance cannot prove
  that a specific camera HAL physically changed torch or zoom, so DroidCast does
  not display an optimistic state or an invented zoom level.
- Camera and mirror-window commands share a single in-flight slot. Requests are
  never retried automatically. A two-second camera timeout disables only camera
  actions for that session; window controls and Stop remain available.
- Zoom requests while the mirrored image is paused return unavailable, matching
  scrcpy's own shortcut behavior. Torch requests remain independent of pause.
- Verification covers all four command bytes, readiness, read-only behavior,
  unavailable responses, timeout isolation, older bridges, contextual UI visibility
  and accessible action names. Real torch/zoom behavior still needs Android 12+
  hardware validation.

This further advances ticket 21. Structured capability selectors and physical
device compatibility testing remain open.

## Part 9: contextual DroidCast artwork

- Added the supplied Desktop and Laptop PNGs to the compiled Qt resources, so
  packaged builds do not depend on repository-relative files at runtime.
- Replaced the generic welcome illustration with the contextual DroidCast artwork.
  Laptop detection uses a real battery on Linux/Windows and a MacBook hardware
  model on macOS; unknown form factors use the Desktop variant rather than guessing
  from screen resolution.
- Crops transparent padding at runtime and scales smoothly within the existing
  welcome hierarchy. The detailed artwork is intentionally not reduced into a
  small toolbar/favicon treatment.
- Preserved the graphite/blue interface palette; teal and green remain contained
  within the authored logo. Both original 2000×2000 assets remain unchanged.
- Added a GCC 16 Release-build workaround after its `-O2`/`-O3` RTL combine pass
  crashed on existing Qt container code. C++ Release targets use `-O1` on GCC 16+ pending
  an upstream compiler fix; other compilers/configurations are unchanged.
- Verification covers resource compilation, automated UI tests and visual review
  at 1240×860. Windows/macOS form-factor detection still needs platform testing.

## Part 10: structured camera capability guidance

- Added a defensive parser for the bounded camera inspection report. It extracts
  camera ID/facing, sensor size, ordinary sizes/FPS, zoom range and declared
  high-speed size/FPS pairs while ignoring unrelated and malformed output.
- Added guided camera, size and frame-rate selectors above the existing advanced
  fields. Selecting a declared value writes through the same validated preference
  path used by manual entry; the raw report remains visible.
- Manual fields remain available because Android declarations may omit working
  values or include combinations that fail. The UI calls them declarations rather
  than guarantees and does not reject an unlisted manual combination.
- High-speed mode changes the guided choices to its declared size/rate pairs.
  Camera/facing choices can still drive automatic selection without forcing an ID.
- Capability data belongs only to the inspected target. Starting another inspection,
  choosing another device, receiving malformed/empty output or losing the target
  clears the guided selectors instead of leaking choices across phones.
- Tests cover normal multi-camera reports, high-speed pairs, malformed/empty input,
  selector write-through, manual fallback and target isolation. Real-device proof
  and Android-version gating remain open.

## Part 11: Android display and system-panel actions

- Extended the opt-in bridge with an explicit `android-controls-ready` capability
  and result events for screen off/on, notifications, Quick Settings, panel collapse,
  Android device rotation and reset video. Older engines never enable the controls.
- Routed every action through scrcpy's existing controller queue on the SDL thread.
  No shell command or synthesized keyboard shortcut is used. A handled response means
  queued, not that Android or an OEM policy honored the request.
- Kept Android device rotation distinct from mirror-window rotation in the API,
  labels, accessibility names and feedback. The session workspace explains the
  difference next to the controls.
- Android actions are hidden for camera sessions and disabled in read-only mode.
  They share one in-flight bridge slot with window and camera commands, use a
  two-second timeout, never retry, and only disable their own family after timeout.
- Added a contextual, keyboard-accessible seven-button group that remains within the
  920×680 layout without horizontal scrolling. Local status reports readiness,
  queued, unavailable and timeout outcomes.
- Verification covers every command byte, real pipe-to-SDL delivery, capability and
  old-engine behavior, wrong mode/read-only, unavailable responses, timeout isolation,
  clean stop and UI context. Physical Android behavior remains in the release-proof
  list.

## Part 12: display and encoder capability foundation

- Added bounded parsers for the existing inspection report's display IDs/sizes and
  video codec/encoder records, including hardware/vendor/alias annotations.
- Parsing begins only at the matching report heading and stops at the next list, so
  malformed lines and similarly shaped data in later sections cannot leak into the
  result.
- Tests cover multiple displays, unknown display size, H.264/H.265/AV1 encoders,
  attributes, malformed/empty lists and section isolation.
- This part intentionally does not change settings UI yet. Guided selectors, manual
  fallback write-through, target-scoped caching and layout/accessibility tests remain
  the next bounded implementation step.

## Part 13: guided display and encoder selection

- Added detected-display and detected-encoder selectors to advanced Video settings.
  Choosing a declaration writes through the existing validated manual control.
- Encoder choices follow the selected H.264/H.265/AV1 codec and retain hardware,
  software, hybrid and vendor annotations from the inspection report.
- Display guidance is disabled for camera and virtual-display modes so a physical
  display ID is not presented as compatible with those capture sources.
- Manual values remain valid and return the guided selector to its neutral entry;
  inspected records are cleared when the selected target changes.
- Tests cover selector population and write-through, codec filtering, manual fallback,
  target isolation and the no-horizontal-scroll contract at 920×680.

## Part 14: searchable installed-app picker

- Added `--list-apps` to the existing asynchronous, selected-device inspection rather
  than introducing an ADB shell parser or a second competing discovery path.
- Added a defensive parser for regular and wrapped scrcpy app-list rows. App name,
  exact package and system-app status remain structured and stop at the next report.
- Added a searchable picker in Virtual display settings. Matching is case-insensitive
  and works anywhere in the visible name/package text; choosing an entry writes the
  exact package through the existing validated `start-app` field.
- The picker is enabled only when virtual display and control are enabled. Manual
  package fallback remains available, inspection data is target-scoped, and changing
  phones clears the list.
- Installed-app contents are shown only in the settings report/picker and are not
  copied into diagnostics. Tests cover malformed/wrapped rows, search configuration,
  selection write-through, read-only dependencies, manual fallback, target isolation
  and the 920×680 layout. Real Android 10+ launch remains release-proof work.

## Part 15: explicit private clipboard actions

- Added separate Copy from Android and Paste to Android buttons to the active-session
  workspace. They are capability-gated, serialized with other bridge requests and
  unavailable in read-only/camera sessions or without keyboard control.
- Reused scrcpy's controller, device-message receiver and SDL host clipboard. The
  private bridge carries only action bytes and status events, never clipboard text.
- Copy completes only after Android returns text and the host clipboard accepts it;
  paste reports that scrcpy queued the existing clipboard/paste control message.
- Redacted injected text and clipboard content from verbose control-message logging;
  logs now retain only operation metadata and byte length.
- Tests cover both bytes, capability/readiness, read-only and camera modes, unavailable
  response, timeout isolation, older engines, accessible UI controls and layout.
  Cross-platform clipboard interoperability remains in release-proof work.
