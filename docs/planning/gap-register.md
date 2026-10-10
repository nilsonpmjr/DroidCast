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
| Session lifecycle | Partial | First-frame state, graceful stop, recording finalization and disconnect/failure states. | Real unplug/reconnect and playable-recording tests on phones. |
| Device workspace | Partial | USB discovery, wireless pair/connect, screenshots, APK install and selected-device targeting. | Complete pair-to-mirror guidance and broader recovery testing. |
| Configuration routes | Partial | Searchable Video, Camera, Audio, Device, Window, Keyboard, Mouse, Gamepad, Recording and Virtual display categories. | Connection/Tunnels, Control, OTG, Video4Linux and complete Shortcuts routes. |
| Toolbar | Partial | Back/Home/Recents, power/volume, screen power, system panels, explicit clipboard copy/paste, inspected-app launch, one-file transfer, Android/window rotation, reset video, mirror-window actions, rendered-FPS measurement and contextual camera controls. | Decide whether multi-file drag/drop and transfer progress belong in release scope. |
| Video | Partial | Codec, bitrate, FPS, size, crop, orientation, guided inspected display/encoder choices, manual fallback and raw report. | Additional codec options, constraint overrides and hardware proof. |
| Audio | Partial | Enablement, source, codec and buffers. | Duplication, encoder/bitrate, require-audio, audio-only and playback policies. |
| Input | Partial | SDK/UHID/disabled keyboard/mouse, key behavior and UHID gamepad. | Mouse bindings, shortcut modifier, AOA modes, gesture help and peripheral tests. |
| Virtual display | Partial | Creation, size/DPI, flex, searchable inspected-app picker, manual package fallback, IME, decorations and close policy. | Android 10+ hardware journeys. |
| Camera | Partial | Source, ID/facing, guided declared size/FPS/high-speed choices, manual fallback, startup torch/zoom and live torch/zoom. | Android-version/capability gates and real camera tests. |
| Recording | Partial | MKV/MP4, rotation, time limit, unique paths and graceful finalization state. | Audio-only/no-playback workflows and playback verification of real files. |
| Embedded video | Open decision | The engine opens a separate native mirror window. | Decide release scope; prototype a real frame inside Qt before promising it. |
| OTG | Open | Nothing end to end. | USB discovery without ADB, control-only session model, drivers and hardware tests. |
| Remote tunnels | Open | Ordinary wireless ADB pairing/connection only. | Consistent remote socket/tunnel configuration across every ADB operation. |
| Linux virtual webcam | Blocked | Upstream feature is known. | Enable V4L2 in the build, detect an existing device, configure it and consume real frames. |
| Distribution | Open | Development staging works on Linux. | Reproducible Linux/Windows/macOS artifacts, notices, Qt deployment, signing and clean-machine QA. |

## Next implementation: complete advanced audio configuration

Bounded one-file transfer is complete in code. The next settings slice should expose
the remaining everyday audio policies without turning the UI into a raw flag editor.

1. Add audio duplication, encoder, bit rate, require-audio and host-playback controls
   to the existing Audio route, using the fork's typed preferences and argument list.
2. Make duplication select the playback source as scrcpy does, explain its Android
   13+ requirement, and prevent contradictory source/control states in the UI.
3. Disable or explain bit rate and encoder fields for raw audio (and ignored bit rate
   cases), while preserving saved values when a temporarily incompatible codec is
   selected.
4. Add an explicit audio-only workflow rather than asking users to infer a combination
   of no-video/no-control/no-window flags. Keep recording and playback consequences
   visible before launch.
5. Extend validation and argument tests for every dependency, invalid numeric/text
   value and persisted round trip. Retain the 920×680 no-horizontal-scroll contract.
6. Hardware-test Android 11 fallback, Android 12 output capture, Android 13 duplication,
   microphone sources, encoder fallback and recordings with host playback disabled.

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
  behavior, not platform clipboard interoperability.
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
- **Cross-platform bridge:** exercise stdin commands and shutdown on Windows; validate
  the app bundle and pipe behavior on macOS.
- **Host branding:** confirm Desktop/Laptop selection on Windows hardware and both
  iMac/MacBook macOS hosts. Linux laptop selection and fallback were reviewed locally.
- **Build toolchain:** GCC 16.2 crashes internally at `-O2`/`-O3`; Release C++
  compilation is capped at `-O1` for GCC 16+. Re-test future GCC releases and remove the workaround
  once the compiler completes the same build reliably.
- **Recording:** open and play finalized MKV/MP4 outputs after normal stop, unplug and
  forced-stop cases.

## How future work updates this file

Every implementation commit should do three things:

1. Move the relevant row only when an end-to-end behavior truly changed.
2. Add remaining manual/hardware proof under “Release-proof still required.”
3. Append technical detail to `implementation-progress.md`; keep this register short
   enough to scan in a few minutes.
