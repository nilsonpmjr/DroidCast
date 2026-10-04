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
| Toolbar | Partial | Back/Home/Recents, power/volume, screen power, system panels, Android/window rotation, reset video, mirror-window actions and contextual camera controls. | Clipboard, app/file actions and measured FPS. |
| Video | Partial | Codec, bitrate, FPS, size, crop, orientation, guided inspected display/encoder choices, manual fallback and raw report. | Additional codec options, constraint overrides and hardware proof. |
| Audio | Partial | Enablement, source, codec and buffers. | Duplication, encoder/bitrate, require-audio, audio-only and playback policies. |
| Input | Partial | SDK/UHID/disabled keyboard/mouse, key behavior and UHID gamepad. | Mouse bindings, shortcut modifier, AOA modes, gesture help and peripheral tests. |
| Virtual display | Partial | Creation, size/DPI, flex, package launch, IME, decorations and close policy. | Structured app picker and Android 10+ hardware journeys. |
| Camera | Partial | Source, ID/facing, guided declared size/FPS/high-speed choices, manual fallback, startup torch/zoom and live torch/zoom. | Android-version/capability gates and real camera tests. |
| Recording | Partial | MKV/MP4, rotation, time limit, unique paths and graceful finalization state. | Audio-only/no-playback workflows and playback verification of real files. |
| Embedded video | Open decision | The engine opens a separate native mirror window. | Decide release scope; prototype a real frame inside Qt before promising it. |
| OTG | Open | Nothing end to end. | USB discovery without ADB, control-only session model, drivers and hardware tests. |
| Remote tunnels | Open | Ordinary wireless ADB pairing/connection only. | Consistent remote socket/tunnel configuration across every ADB operation. |
| Linux virtual webcam | Blocked | Upstream feature is known. | Enable V4L2 in the build, detect an existing device, configure it and consume real frames. |
| Distribution | Open | Development staging works on Linux. | Reproducible Linux/Windows/macOS artifacts, notices, Qt deployment, signing and clean-machine QA. |

## Completed slice: guided display and encoder capabilities

The parser and configuration UI are implemented. Like the camera work,
declarations guide valid choices without pretending they guarantee runtime support.

1. **Done:** parse display IDs/sizes and codec-grouped video encoders from the
   bounded inspection report, stopping cleanly at unrelated sections.
2. **Done:** keep the raw report and manual values available for incomplete vendor output.
3. **Done:** populate a display selector without confusing a physical display with a new
   virtual display; choosing one must update the existing validated `display-id`.
4. **Done:** group encoder choices by codec and show only encoders compatible with the chosen
   H.264/H.265/AV1 codec while preserving an unlisted manual encoder value.
5. **Done:** cache declarations only for the selected connection and clear them on target
   change, failed inspection or an empty/malformed report.
6. **Done:** keep camera-mode dependencies intact: display choice is disabled for camera, while
   camera guidance remains isolated from display/encoder guidance.
7. **Done:** test multi-display/multi-codec, malformed output, manual fallback, target isolation,
   keyboard access and 920×680 layout.
8. Verify at least one hardware encoder and a non-primary display on real devices;
   record rejected choices as runtime evidence rather than hiding them.

## Release-proof still required for recent work

- **Virtual display:** Android 10+ creation, typing, flex resize, app start and close
  policy on hardware, including a phone with no virtual-display launcher.
- **Camera:** Android 12+ front/back selection, recording, high-speed failure/success,
  torch availability and zoom limits. A bridge result currently means “request queued,”
  not “camera hardware confirmed the change.”
- **Android actions:** verify screen power, notification/Quick Settings panels,
  panel collapse, device rotation and reset-video on multiple Android versions.
  Queue acknowledgement does not prove that Android honored the request.
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
