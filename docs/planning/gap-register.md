# DroidCast Desktop: gap register

This is the short operational view of the project. Read this file first when the
implementation progress is hard to follow. The ticket draft keeps the complete
acceptance criteria; this register says what is usable now and what must happen next.

## Status language

- **Implemented:** code and automated tests exist. Hardware/platform verification
  may still be listed separately.
- **Partial:** a real path exists, but important behavior or proof is missing.
- **Open:** no end-to-end product path exists yet.
- **Blocked:** another named item must be completed first.

An automated fake-process test proves orchestration, validation and failure handling.
It does **not** prove that a phone, camera, codec or operating-system package works.

## Current checkpoint

| Area | Status | What works now | Gap to close |
| --- | --- | --- | --- |
| Bundled runtime | Partial | The forked client, ADB and matching server are launched without executable pickers. | Clean-machine packages, signing and Windows/macOS verification. |
| Session lifecycle | Partial | First-video-frame or first-audio-packet state, graceful stop, recording finalization and disconnect/failure states. | Real unplug/reconnect and playable-recording tests on phones. |
| Device workspace | Partial | USB discovery, wireless pair/connect, screenshots, APK install and selected-device targeting. | Complete pair-to-mirror guidance and broader recovery testing. |
| Configuration routes | Partial | Connection/Tunnels plus searchable Video, Camera, Audio, Control, Device, Window, Keyboard, Mouse, Gamepad, Shortcuts, Recording and Virtual display routes. | OTG and Video4Linux routes, plus remaining advanced option coverage. |
| Toolbar | Partial | Back/Home/Recents, power/volume, screen power, system panels, explicit clipboard copy/paste, inspected-app launch, one-file transfer, Android/window rotation, reset video, mirror-window actions, rendered-FPS measurement and contextual camera controls. | Decide whether multi-file drag/drop and transfer progress belong in release scope. |
| Video | Partial | Codec, bitrate, FPS, size, crop, orientation, guided inspected display/encoder choices, manual fallback and raw report. | Additional codec options, constraint overrides and hardware proof. |
| Audio | Partial | Enablement, all capture sources, Android playback duplication, codec-matched inspected/manual encoder, bitrate, require-audio, host playback/buffers, first-packet-backed audio-only sessions and codec-safe dedicated recording formats. | Physical-device output and playable-file proof. |
| Input | Partial | SDK/UHID/disabled keyboard/mouse, key behavior, UHID gamepad, validated shortcut modifier, complete shortcut/gesture reference, clipboard policies and mirror-drop target. | Mouse bindings, AOA modes and real peripheral/clipboard/drop tests. |
| Virtual display | Partial | Creation, size/DPI, flex, searchable inspected-app picker, manual package fallback, IME, decorations and close policy. | Android 10+ hardware journeys. |
| Camera | Partial | Source, ID/facing, guided declared size/FPS/high-speed choices, manual fallback, startup torch/zoom and live torch/zoom. | Android-version/capability gates and real camera tests. |
| Recording | Partial | Mode-specific MKV/MP4/MKA/M4A/OPUS/AAC/FLAC/WAV, codec validation, migration, unique paths, filters and graceful finalization state. | Playback verification of real files after normal and abnormal endings. |
| Embedded video | Open decision | The engine opens a separate native mirror window. | Decide release scope; prototype a real frame inside Qt before promising it. |
| OTG | Open | Nothing end to end. | USB discovery without ADB, control-only session model, drivers and hardware tests. |
| Remote tunnels | Partial | Local/remote ADB route, forced-forward tunnel options, persistence and one transport snapshot across every ADB/scrcpy child are covered by automated tests. | Real remote ADB and SSH tunnel journeys, unreachable-host recovery review and Windows/macOS proof. |
| Linux virtual webcam | Blocked | Upstream feature is known. | Enable V4L2 in the build, detect an existing device, configure it and consume real frames. |
| Distribution | Open | Development staging works on Linux. | Reproducible Linux/Windows/macOS artifacts, notices, Qt deployment, signing and clean-machine QA. |

## Connection and Tunnels checkpoint

The product path and automated boundary proof now exist:

1. Local versus remote ADB is explicit. Remote `host:port` becomes
   `ADB_SERVER_SOCKET=tcp:host:port`; local mode removes any inherited socket.
2. Discovery, pairing/connect, capability inspection, screenshot, installation, file
   transfer, phone actions and scrcpy all receive the same environment. A running
   session pins its route even if next-session preferences change.
3. Validated `--tunnel-host`/`--tunnel-port` values work with either transport.
   Automatic/reverse-first behavior remains scrcpy's default; forced-forward mode
   adds `--force-adb-forward` for an existing SSH `-L` data tunnel.
4. Endpoints and policy persist, but pairing codes, SSH credentials and secrets do not.
   The app warns that raw remote ADB is unencrypted and never constructs SSH commands.
5. Automated tests cover validation, arguments, every child-process family, inherited-
   environment isolation, route switching and the 920×680 layout.

Before promoting this row from Partial to Implemented, run a real local journey, a
controlled remote ADB journey and the applicable SSH forwarding topologies on at least
two desktop platforms. Record behavior for unreachable servers and switching back to
local after timeout; the current fake-process proof cannot validate network topology.

## Control and Shortcuts checkpoint

The searchable Control route now groups read-only, clipboard synchronization,
compatibility paste and a validated mirror-window drop destination. Explicit
DroidCast transfer remains separate and labelled. Shortcuts has a validated modifier,
a direct route from Input & controls and the complete fork action/gesture reference.
Argument, persistence, inline-validation, accessibility and 920×680 tests exist.

Manual proof still required: Unicode clipboard and legacy paste on representative
Android versions; APK/non-APK drag/drop into the native mirror; modifier conflicts and
Windows/macOS Super-key naming. The toolbar classification remains documented in the
feature map and its mutating paths remain covered by existing read-only tests.

## Next implementation: advanced mouse and AOA input

1. Add validated primary/Shift mouse binding sequences with a visual four-button map.
2. Expose AOA keyboard, mouse and gamepad only for compatible USB sessions; do not
   present it as working over wireless ADB.
3. Explain SDK capture, UHID cursor behavior, secondary-click forwarding and recovery
   when a phone or desktop driver rejects a physical input mode.
4. Add argument/dependency/layout coverage, then verify real mice, keyboards and
   controllers over USB on Linux and Windows before starting the separate OTG mode.

## Release-proof still required for recent work

- **Virtual display:** Android 10+ creation, typing, flex resize, app start and close
  policy on hardware, including a phone with no virtual-display launcher.
- **Camera:** Android 12+ front/back selection, recording, high-speed failure/success,
  torch availability and zoom limits. A bridge result currently means “request queued,”
  not “camera hardware confirmed the change.”
- **Android actions:** verify screen power, notification/Quick Settings panels,
  panel collapse, device rotation and reset-video on multiple Android versions.
  Queue acknowledgement does not prove that Android honored the request.
- **Clipboard:** verify copy and paste with empty, Unicode and maximum practical
  content on Android/Linux, Windows and macOS. Automated tests prove command/privacy
  behavior and policy arguments, not platform clipboard interoperability. Also verify
  compatibility paste with non-ASCII text and automatic synchronization disabled.
- **Shortcuts and mirror drop:** verify custom modifiers and Super-key conflicts on
  each desktop platform; drop APK/non-APK files on the native mirror and confirm the
  configured Android destination. Automated tests only prove arguments and UI routes.
- **Rendered FPS:** compare structured samples with scrcpy's ordinary FPS log on
  display, read-only and camera sessions; verify skipped-frame reporting under load
  and clean stop/disconnect behavior on all desktop platforms.
- **Live app launch:** verify primary, selected secondary and newly created virtual
  displays on multiple Android versions, including a package with no launchable
  activity. A queue acknowledgement does not prove foreground launch.
- **File transfer:** push small and multi-gigabyte files, filenames and destinations
  with spaces/Unicode, replacement of an existing filename, cancellation and storage-
  permission failures over USB and wireless ADB on Linux, Windows and macOS. Automated
  tests prove command construction and state handling, not bytes written by a phone.
- **Advanced audio:** exercise every source family that the phone permits, Android 13+
  duplication, inspected/default encoder fallback, custom bitrates, required-audio
  failures and recording with computer playback disabled. Vendor audio policies and
  routes cannot be proven by the fake-process tests.
- **Audio-only:** verify Android 11 unlocked-start behavior, Android 12+ playback and
  microphone sources, Android 13 duplication, Bluetooth/routing changes, playback-
  disabled recording, disconnect and long sessions. Automated tests prove packet-
  backed state and mode boundaries, not audible output or a playable file.
- **Cross-platform bridge:** exercise stdin commands and shutdown on Windows; validate
  the app bundle and pipe behavior on macOS.
- **Remote connection:** use a controlled remote ADB server and real SSH forwarding;
  verify discovery-to-session consistency, unreachable-server recovery and return to
  local mode on Linux plus at least one other desktop platform.
- **Host branding:** confirm Desktop/Laptop selection on Windows hardware and both
  iMac/MacBook macOS hosts. Linux laptop selection and fallback were reviewed locally.
- **Build toolchain:** GCC 16.2 crashes internally at `-O2`/`-O3`; Release C++
  compilation is capped at `-O1` for GCC 16+. Re-test future GCC releases and remove the workaround
  once the compiler completes the same build reliably.
- **Recording:** open and play finalized MKV/MP4 video plus every supported dedicated
  audio format after normal stop, unplug, playback-disabled capture and forced stop.

## How future work updates this file

Every implementation commit should do three things:

1. Move the relevant row only when an end-to-end behavior truly changed.
2. Add remaining manual/hardware proof under “Release-proof still required.”
3. Append technical detail to `implementation-progress.md`; keep this register short
   enough to scan in a few minutes.
