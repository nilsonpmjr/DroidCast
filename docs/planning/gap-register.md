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
| Toolbar | Partial | Back/Home/Recents, power, volume, mirror-window actions and contextual camera controls. | Panels, clipboard, display power, app/file actions, reset video and measured FPS. |
| Video | Partial | Codec, bitrate, FPS, size, crop, orientation, display ID, encoder and inspection. | Codec options, constraint overrides, structured capability selection and hardware proof. |
| Audio | Partial | Enablement, source, codec and buffers. | Duplication, encoder/bitrate, require-audio, audio-only and playback policies. |
| Input | Partial | SDK/UHID/disabled keyboard/mouse, key behavior and UHID gamepad. | Mouse bindings, shortcut modifier, AOA modes, gesture help and peripheral tests. |
| Virtual display | Partial | Creation, size/DPI, flex, package launch, IME, decorations and close policy. | Structured app picker and Android 10+ hardware journeys. |
| Camera | Partial | Source, ID/facing, size/aspect, FPS/high-speed, startup torch/zoom and live torch/zoom. | Parse inspection into selectors, Android-version/capability gates and real camera tests. |
| Recording | Partial | MKV/MP4, rotation, time limit, unique paths and graceful finalization state. | Audio-only/no-playback workflows and playback verification of real files. |
| Embedded video | Open decision | The engine opens a separate native mirror window. | Decide release scope; prototype a real frame inside Qt before promising it. |
| OTG | Open | Nothing end to end. | USB discovery without ADB, control-only session model, drivers and hardware tests. |
| Remote tunnels | Open | Ordinary wireless ADB pairing/connection only. | Consistent remote socket/tunnel configuration across every ADB operation. |
| Linux virtual webcam | Blocked | Upstream feature is known. | Enable V4L2 in the build, detect an existing device, configure it and consume real frames. |
| Distribution | Open | Development staging works on Linux. | Reproducible Linux/Windows/macOS artifacts, notices, Qt deployment, signing and clean-machine QA. |

## Next implementation: structured camera capabilities

This is the next bounded slice. It should not claim hardware support merely because
Android listed a value.

1. Parse the bounded `--list-camera-sizes` report into camera records: ID, facing,
   declared sizes, ordinary frame rates, high-speed size/rate pairs and zoom range.
2. Keep the raw report available for diagnostics when parsing is incomplete or a
   vendor changes formatting.
3. Replace free-text camera ID with a selector when records exist; retain a manual
   entry fallback so valid but undeclared vendor IDs are still possible.
4. Populate size and FPS selectors from the selected camera. High-speed mode must
   show only declared size/rate pairs instead of independent misleading choices.
5. Preserve current mutual-exclusion rules: ID versus facing, and exact size versus
   general size/aspect constraints.
6. Cache results only for the current device connection. Clear them on disconnect,
   target change or failed inspection.
7. Test normal, incomplete, malformed and empty reports, target isolation, keyboard
   access and the 920×680 layout.
8. Verify on at least one Android 12+ phone. Record which camera/size/FPS combinations
   actually start, including one unsupported combination and torch/zoom behavior.

## Release-proof still required for recent work

- **Virtual display:** Android 10+ creation, typing, flex resize, app start and close
  policy on hardware, including a phone with no virtual-display launcher.
- **Camera:** Android 12+ front/back selection, recording, high-speed failure/success,
  torch availability and zoom limits. A bridge result currently means “request queued,”
  not “camera hardware confirmed the change.”
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
