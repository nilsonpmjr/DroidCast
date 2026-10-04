# DroidCast: scrcpy configuration and live-control coverage

Status: planning inventory, not a claim that these controls are implemented.
Source of truth: the checked-out scrcpy 4.1 CLI definitions, control-message types
and the sixteen documentation topics shown in the user's screenshot.

## Navigation

Keep the approved colors, workspace sidebar and device/session pages. Expand Settings
with a searchable category navigator; each documented topic has a direct route.
Search finds plain-language terms and their CLI flag names. Advanced options live
within their category, not in an unvalidated free-form command box.

| Documentation topic | Proposed destination | Controls and workflows |
| --- | --- | --- |
| Connection | Devices / Connection | Device selection; USB and wireless; pairing; automatic USB-to-TCP/IP setup; connection retry |
| Video | Settings / Video | Source, display, dimensions, bitrate, FPS, codec/encoder, orientation, crop, buffering, playback |
| Audio | Settings / Audio | Source, duplication, codec/encoder, bitrate, buffering, playback, audio-only mode |
| Control | Settings / Control; session tools | Read-only mode, clipboard policy, paste, touch gestures, file transfer and APK installation |
| Keyboard | Input / Keyboard | SDK, UHID, AOA, disabled; text/key injection; repeat; Android keyboard settings |
| Mouse | Input / Mouse | SDK, UHID, AOA, disabled; hover; button bindings; capture behavior |
| Gamepad | Input / Gamepad | Disabled, UHID and AOA; compatibility and connected-controller guidance |
| Device | Settings / Device; session tools | Keep-active, stay-awake, screen timeout, screen power, show touches, launch app |
| Window | Settings / Window; session view controls | Title, position, dimensions, aspect lock, background, border, fullscreen, render fit, screensaver |
| Recording | Captures / Recording settings | Destination, format, video/audio-only, orientation, playback policy, duration limit |
| Virtual display | Session mode / Virtual display | Size/DPI, flex resizing, app selection, decorations, content lifecycle and IME policy |
| Tunnels | Settings / Connection / Advanced | Remote ADB socket, tunnel host/port, forward mode; connect through an existing tunnel |
| OTG | Session mode / USB control only | USB HID operation without ADB authorization; keyboard/mouse/gamepad selection; no video/audio |
| Camera | Session mode / Camera | Camera list, facing/ID, size/aspect, FPS/high-speed, orientation, torch and zoom |
| Video4Linux | Settings / Outputs / Linux webcam | V4L2 device and buffering; camera/display source; prerequisite checks |
| Shortcuts | Input / Shortcuts; toolbar tooltips | Modifier selection, complete action reference and accessible action buttons |

## Option inventory

Every long option declared by this fork must appear below. Implementers classify each
as a user control, a capability query or an application-managed policy. All three are
legitimate coverage; a GUI equivalent of `--help` is help, not a meaningless toggle.
Short aliases inherit the same mapping. Session actions not represented by flags are
listed separately below.

| Category | CLI options accounted for | Exposure |
| --- | --- | --- |
| Connection | `--serial`, `--select-usb`, `--select-tcpip`, `--tcpip` | Device selectors and explicit connection actions |
| Tunnels and transport | `--port`, `--tunnel-host`, `--tunnel-port`, `--force-adb-forward` | Advanced connection fields, validated as a coherent configuration |
| Video stream | `--video-source`, `--max-size`, `--max-fps`, `--video-bit-rate`, `--video-codec`, `--video-encoder`, `--video-codec-options`, `--video-buffer` | Basic controls with encoder-specific advanced fields |
| Video geometry | `--capture-orientation`, `--display-orientation`, `--orientation`, `--angle`, `--crop`, `--display-id` | Separate captured-image, presentation and device-orientation concepts |
| Video compatibility | `--min-size-alignment`, `--ignore-video-encoder-constraints`, `--no-downsize-on-error` | Advanced controls with explanatory compatibility notes |
| Audio | `--audio-source`, `--audio-dup`, `--audio-codec`, `--audio-encoder`, `--audio-codec-options`, `--audio-bit-rate`, `--audio-buffer`, `--audio-output-buffer`, `--require-audio` | Audio settings; unavailable sources explained rather than promised |
| Stream/playback modes | `--no-video`, `--no-audio`, `--no-playback`, `--no-video-playback`, `--no-audio-playback`, `--no-window` | Explicit session modes; validate useful combinations rather than independent contradictory toggles |
| Control and clipboard | `--no-control`, `--no-clipboard-autosync`, `--legacy-paste`, `--push-target` | Read-only, synchronization policy, paste compatibility and transfer destination |
| Keyboard | `--keyboard`, `--prefer-text`, `--raw-key-events`, `--no-key-repeat` | Input mode and injection behavior |
| Mouse and gamepad | `--mouse`, `--mouse-bind`, `--no-mouse-hover`, `--gamepad` | Input modes and supported mappings |
| Shortcuts | `--shortcut-mod` | Host modifier configuration and live action hints |
| Device behavior | `--keep-active`, `--stay-awake`, `--turn-screen-off`, `--screen-off-timeout`, `--show-touches`, `--power-off-on-close`, `--no-power-on`, `--start-app` | Device/session behavior; describe screen power accurately, not as shutting down Android |
| Window | `--window-title`, `--window-x`, `--window-y`, `--window-width`, `--window-height`, `--window-borderless`, `--always-on-top`, `--fullscreen`, `--background-color`, `--no-window-aspect-ratio-lock`, `--disable-screensaver`, `--render-fit`, `--render-driver`, `--no-mipmaps` | Window/presentation settings; distinguish app chrome from the engine's display surface |
| Recording | `--record`, `--record-format`, `--record-orientation`, `--time-limit` | Capture settings and recording lifecycle; format choices constrained by selected streams/codecs |
| Virtual display | `--new-display`, `--flex-display`, `--no-vd-system-decorations`, `--no-vd-destroy-content`, `--display-ime-policy` | Virtual-display mode; reject combinations incompatible with physical display or camera selection |
| OTG | `--otg` | Distinct USB-only control session; does not require ADB-ready device state |
| Camera | `--camera-id`, `--camera-facing`, `--camera-size`, `--camera-ar`, `--camera-fps`, `--camera-high-speed`, `--camera-torch`, `--camera-zoom` | Camera mode; validated camera-specific controls |
| Linux webcam output | `--v4l2-sink`, `--v4l2-buffer` | Linux-only output settings with engine/build and kernel-device checks |
| Capability discovery | `--list-apps`, `--list-cameras`, `--list-camera-sizes`, `--list-displays`, `--list-encoders` | Populate choices through asynchronous queries, not standalone user toggles |
| Diagnostics and help | `--help`, `--version`, `--verbosity`, `--print-fps` | Help/About, logging preferences and measured diagnostics; no invented FPS values |
| Engine lifecycle policy | `--kill-adb-on-close`, `--no-cleanup`, `--no-terminal-title`, `--pause-on-exit` | Document app-managed defaults. Do not kill a shared ADB server or leave device changes behind by default; terminal-only behavior belongs in diagnostics/developer policy |

Also cover the documented `ADB_SERVER_SOCKET` environment setting in advanced
connection settings. Bundled executable/server environment overrides remain internal
to runtime management. Autostart recipes become desktop launch preferences/workflows
only after their behavior is specified; they are not permission to auto-mirror.

## Android Studio–style session toolbar

Use a compact toolbar beside the live session surface, with accessible button names,
keyboard shortcuts, tooltips and an overflow panel. Preserve enough space for the
phone display; do not expose every configuration field as a permanent toolbar button.
These commands must also work when the mirror remains in a separate window.

| Group | Controls | Behavioral requirement |
| --- | --- | --- |
| Android navigation | Back, Home, Recents, Menu | Send to the intended live session; disable for read-only or incompatible modes |
| Device power and sound | Power key, volume up/down, phone screen off/on | Distinguish key press from display-power command; do not imply shutdown/reboot |
| Android panels | Notifications, Quick settings, collapse panels | Route to the phone via its control channel |
| Rotation | Rotate Android device; separately rotate mirror left/right and flip horizontal/vertical | Label phone rotation and visual transformation distinctly |
| Mirror view | Fullscreen, fit, 1:1, pause/resume display | Pausing presentation does not imply pausing Android or recording; explain the actual effect |
| Capture | Screenshot; record/start with recording; stop and finalize recording/session | Runtime recording start/pause is not an existing CLI guarantee; show restart-required behavior until implemented end to end |
| Clipboard | Copy, cut, paste, inject text; synchronization preference | Honor Android/version/input restrictions; clipboard data never goes into diagnostics |
| Apps and files | Launch app, install APK, send file, open captures | Explicit device target; asynchronous progress and truthful results |
| Input tools | Open Android physical-keyboard settings; gesture help for pinch/rotate/tilt | Expose only supported input modes; gestures have documentation and appropriate alternatives |
| Engine tools | Reset video, show measured FPS, stop session | Commands go to the correct engine instance and return useful success/failure status |
| Camera tools | Torch on/off, zoom in/out | Visible/enabled for a camera session and supported hardware only |

## Integration and availability rules

1. Classify every control as **before starting**, **live**, **query**, or **managed by DroidCast**. A setting changing in the UI must never falsely imply the running engine changed.
2. Use the fork's existing Android control messages and local renderer actions behind an explicit desktop-to-engine command interface. Do not automate global keyboard shortcuts to control the mirror; focus and platform differences would make that unreliable.
3. Android Studio is an interaction reference. Emulator-only facilities such as simulated GPS, battery, cellular conditions, snapshots and arbitrary sensors are not promised for a physical phone by this scrcpy inventory.
4. Read-only mode must block every device-mutating route, including toolbar actions, clipboard writes, APK installation and file push. Rendering-only actions can remain available.
5. Model modes explicitly: display mirror, virtual display, camera, audio only, ADB control only and OTG. OTG needs its own USB discovery path; it must not be blocked by the current authorized-ADB-device check.
6. Validate platform, Android version, transport, engine build and device capabilities. Camera-size versus max-size/aspect conflicts, disabled streams, unsupported encoders, AOA-over-wireless and V4L2-on-non-Linux all need explanations before launch.
7. V4L2 is currently disabled in the desktop engine build. The Linux webcam ticket must enable/package the engine capability and check the external virtual-device prerequisite. Adding fields alone is insufficient.
8. Support existing trusted remote ADB/tunnel endpoints without automatically exposing a local ADB server on the network or managing SSH credentials in the first iteration.
9. Give advanced codec options structured key/value editing and validation. Launch with argument arrays; do not hand user input to a shell.
10. Each completed capability needs UI reachability, validated configuration, correct engine behavior, saved-preference behavior where appropriate, capability feedback, and behavioral tests.

## Coverage tracking

For each option/action, implementation work should record its destination, label,
default, value/unit, applicability, launch/live classification, persistence policy,
validation, controller binding and tests. Compare the inventory against the fork's
actual CLI on engine upgrades. A deliberately managed option must have a documented
policy; it cannot simply disappear from the coverage report.

The matrix above is the scope baseline. It does not make all features equally urgent:
daily device controls and standard video/audio settings should arrive before specialist
camera/virtual-display/OTG/tunnel/output workflows, with the latter retained in scope.
