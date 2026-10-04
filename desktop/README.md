# DroidCast Desktop

Project status and remaining work are summarized in the
[gap register](../docs/planning/gap-register.md); the longer planning documents
retain specifications, tickets and chronological implementation notes.

A native C++17 / Qt 6 workspace built around this scrcpy fork. DroidCast includes
its engine, ADB and Android server. Users open the app, connect a phone and start
mirroring; there is no executable picker or separate scrcpy installation step.

## Build and launch

Developer prerequisites: C++17 compiler, CMake 3.21+, Qt 6.4+ Widgets and Test,
Meson, Ninja, and the scrcpy client development dependencies from
[the build documentation](../doc/build.md). On this Linux checkout those include
SDL3, FFmpeg (avcodec, avformat, avutil, swresample), and libusb.

```sh
cmake -S desktop -B build-droidcast -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-droidcast --parallel 4
ctest --test-dir build-droidcast --output-on-failure
./build-droidcast/droidcast-desktop
```

CMake downloads the pinned scrcpy 4.1 platform archive and verifies SHA-256
against the checksums already recorded in this repository. It takes ADB, the
matching prebuilt Android server and supporting resources from that archive,
builds the C client **from this checkout**, and places everything in `runtime/`
beside DroidCast. The application performs no downloads at launch. The first
configure needs internet; verified archives are reused from `build-droidcast/downloads`.

The prebuilt server is appropriate for the current, unmodified Java server. If
you change the server/protocol, build and stage a matching server too; do not mix
different client/server versions. The Gradle server build is not yet integrated
into the desktop build.

The build understands Linux x86_64, Windows x64, and macOS x86_64/arm64 runtime
archives. Qt and engine development dependencies must be available on the build
machine. For UI-only development, `-DDROIDCAST_BUILD_FORK=OFF` uses the verified
upstream 4.1 engine from the bundle, without first-frame reporting or the private
graceful-stop protocol; it is not suitable for release. `-DDROIDCAST_BUNDLE_RUNTIME=OFF` is for isolated
tests only; that build is not a distributable app. `-DBUILD_TESTING=OFF` omits Qt Test.

For multi-configuration generators add `--config Release`, then launch
`build-droidcast/Release/droidcast-desktop.exe` on Windows. On macOS open the generated
`droidcast-desktop.app`; runtime files are inside `Contents/MacOS/runtime`.

```sh
cmake --install build-droidcast --prefix build-droidcast/package
```

On Linux/Windows, this stages the executable and runtime under `package/bin`.
On macOS it installs the application bundle. This is **development staging**, not
a standalone installer: Qt runtime deployment, shared libraries for the locally
built engine, distribution notices, signing/notarization, and platform QA remain
release work. Linux build, bundle resolution and tests have been verified locally;
Windows/macOS build paths have not yet been exercised.

## Workspace

- **Connected devices:** illustrated first-connection guidance, automatic ADB
  discovery, device cards, authorization/USB-permission states, explicit device
  selection, quick session preferences, mirror and record actions.
- **Active session:** first-frame-backed lifecycle state, elapsed process time,
  selected phone, recording destination, screenshot capture, APK installation,
  Back/Home/Recents/Power/volume toolbar and graceful stop. The live video opens in
  a separate DroidCast mirror window; the workspace illustration is not a preview.
  Window controls provide fullscreen, fit/pixel-perfect sizing, rotation and
  pause/resume through the fork. Pausing freezes only the computer image; audio
  and recording continue. These host-only controls also work in read-only mode.
  Display sessions add explicit Android screen on/off, notification, Quick
  Settings, collapse-panels, device rotation and video-reset actions. They are
  capability-gated, serialized and disabled in read-only or camera sessions.
  Camera sessions reveal contextual torch on/off and relative zoom controls.
  They use scrcpy's Android control channel, so they are unavailable in read-only
  mode and report requests rather than assuming the hardware accepted them.
- **Wireless pairing:** six-digit pairing code and separate pairing/connection
  addresses. Codes go through stdin and are never saved.
- **Recordings & captures:** actual local PNG, MKV and MP4 files, type filters,
  sizes/dates, open folder and open capture. An active recording cannot be opened
  until the session ends.
- **Input & controls:** SDK/UHID/disabled keyboard and mouse choices, accurate
  shortcut reference using scrcpy's default left Alt / left Super modifier, and
  a route to advanced input settings including UHID gamepad forwarding.
- **Settings:** searchable categories for video, camera, audio, device, window,
  keyboard, mouse, gamepad, recording and virtual displays. Includes read-only
  mode, buffering, orientation,
  SDK input preferences, MKV/MP4 recording, recording rotation and session time
  limit. Settings apply to the next session and persist locally. Options that do
  not apply to the chosen input mode are inactive, but their saved values remain.
  Advanced video includes crop, capture orientation/locking, Android display ID,
  encoder name and automatic-downsize opt-out. Inspect an authorized phone while
  idle to read its real display/encoder report, then copy the desired ID or name.
  Inspection has cancellation and a timeout; it does not start a mirror.
  The **Virtual display** category adds Android 10+ display creation, resolution/
  density, flexible resizing, keyboard placement, system decorations and app
  handling on close. An optional exact Android package starts on that display;
  no search or force-stop prefix is accepted. Some phones have no launcher there,
  so leaving the package empty may produce no video. These changes take effect
  only after explicitly starting a session.
  The **Camera** category adds Android 12+ camera capture with source, exact ID or
  facing, exact size or aspect ratio, frame rate, high-speed mode, torch and zoom.
  Dependent controls prevent combinations rejected by scrcpy. Inspect an idle
  phone to list its declared cameras, sizes and rates; Android camera declarations
  may still be incomplete or inaccurate, so hardware remains the final check.
- **Diagnostics:** bounded in-memory output from the engine and device services.

Recordings go directly to the chosen capture directory (default: Videos/DroidCast)
with unique filenames. Screenshots are validated as PNG before atomic saving. APK
installation runs only after choosing a file; an install failure is reported even
if ADB exits with code zero. There is no pretend battery, latency or device data.

Phone-toolbar commands and ADB screenshots currently target the primary display.
They are disabled for an active secondary/virtual display so they cannot silently
act on the wrong screen. Use input inside the mirror and session recording there.
They are also disabled for active camera capture, because neither route targets
the camera stream. Use session recording to save camera video.
Host-window controls remain available; in flexible-display mode, resizing also
changes Android's display. Read-only mode disables flexible resizing and app
launch, but creating an explicitly selected virtual display still occurs.

Close the mirror window or use Stop to request normal engine cleanup. The fork
reports recorder finalization separately from process completion. An unresponsive
process is killed after five seconds and the app warns that recordings may be
incomplete. Streaming is reported only after the first video frame, not merely
process startup. See [the private bridge documentation](BRIDGE.md).

## Verification and visual review

Tests cover device parsing and states, argument boundaries, wireless validation,
settings persistence, no automatic mirroring, process failures/timeouts, pairing,
valid/invalid screenshot handling, APK results, synchronized preference switches,
input-mode compatibility, recording validation, first-frame lifecycle, graceful
stop/finalization, disconnects, and absence of executable-path settings. Tests use a compiled helper and temporary
runtime; no connected phone is required and no fake devices enter the product.

```sh
QT_QPA_PLATFORM=offscreen ./build-droidcast/droidcast-desktop --screenshot /tmp/droidcast-devices.png
QT_QPA_PLATFORM=offscreen ./build-droidcast/droidcast-desktop --page 5 --screenshot /tmp/droidcast-settings.png
```

For test/developer injection only, `DROIDCAST_RUNTIME_DIR` changes the bundle root.
Normal resolution is relative to the application, independent of PATH or working
directory. Old user-selected tool paths are not loaded or saved.

## Design and next work

The AIStudio TypeScript project informs the navigation, device cards, session
tools, capture library, and grouped controls. The requested blue-gray palette is
retained: `#171d25` sidebar, `#20252d` workspace, `#edf1f7` text, `#b4c0d0`
secondary text, `#245bc2` actions and `#91bbff` focus. Native platform title bars,
vector icons, keyboard-operable switches and constrained content widths keep the
layout usable across desktop sizes. The welcome state uses the supplied DroidCast
artwork: the iMac variant for desktop hosts and the MacBook variant when a laptop
can be identified from host power/model information. Unknown form factors safely
fall back to Desktop. The original TS project remains untouched.

Next: validate display, virtual-display, Android actions and camera sessions with
physical devices; turn inspected display/encoder declarations into guided choices;
integrate live video into the workspace; add per-device profiles and concurrent
sessions; then finish standalone platform installers.
