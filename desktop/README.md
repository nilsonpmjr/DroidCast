# DroidCast Desktop

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
upstream 4.1 engine from the bundle. `-DDROIDCAST_BUNDLE_RUNTIME=OFF` is for isolated
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
- **Active session:** elapsed process time, selected phone, recording destination,
  screenshot capture, APK installation and stop control. The live video opens in
  a separate DroidCast mirror window; the workspace illustration is not a preview.
- **Wireless pairing:** six-digit pairing code and separate pairing/connection
  addresses. Codes go through stdin and are never saved.
- **Recordings & captures:** actual local PNG, MKV and MP4 files, type filters,
  sizes/dates, open folder and open capture. An active recording cannot be opened
  until the session ends.
- **Input & controls:** SDK/UHID keyboard and mouse choices, accurate shortcut
  reference using scrcpy's default left Alt / left Super modifier.
- **Settings:** resolution, FPS cap, bitrate, video codec, audio, screen behavior
  and capture directory. Settings apply to the next session and persist locally.
- **Diagnostics:** bounded in-memory output from the engine and device services.

Recordings go directly to the chosen capture directory (default: Videos/DroidCast)
with unique filenames. Screenshots are validated as PNG before atomic saving. APK
installation runs only after choosing a file; an install failure is reported even
if ADB exits with code zero. There is no pretend battery, latency or device data.

Close the mirror window to finish recordings cleanly. Stop requests process
termination and kills an unresponsive process after five seconds. Especially on
Windows, forced termination can leave a recording incomplete. A process starting
is not proof that streaming has begun; connection errors appear in Diagnostics.

## Verification and visual review

Tests cover device parsing and states, argument boundaries, wireless validation,
settings persistence, no automatic mirroring, process failures/timeouts, pairing,
valid/invalid screenshot handling, APK results, synchronized preference switches,
and absence of executable-path settings. Tests use a compiled helper and temporary
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
layout usable across desktop sizes. The original TS project remains untouched.

Next: validate with physical devices; add engine IPC for graceful shutdown and
reliable streaming events; integrate live video into the workspace; add per-device
profiles and concurrent sessions; then finish standalone platform installers.
