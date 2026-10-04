# DroidCast Desktop: complete desktop workspace

Status: draft for review, not published to the issue tracker.

Baseline: `82db2c67`, the approved native interface and bundled-engine implementation.
Prepared using the installed `to-spec` skill. The proposed test boundary and release
scope still need review. GitHub Issues in nilsonpmjr/DroidCast is the proposed tracker;
`ready-for-agent` is the proposed label after approval.

## Problem Statement

The scrcpy engine starts mirroring without providing a complete desktop workspace.
People need a welcoming app that helps them connect a phone, understand its state,
control a session, configure quality and input, and manage captures. They should not
have to install or locate scrcpy, ADB or a matching Android server themselves.

The existing AIStudio design demonstrated the desired breadth of the workspace,
but its devices and actions were simulated. DroidCast now has a real native shell,
and the user has approved its current interface and blue-gray palette. The remaining
problem is turning that working development build into a dependable desktop product
for Windows, Linux and macOS, with a route that distinguishes proven behavior from
unfinished integration and release work.

## Solution

Ship DroidCast Desktop as one installed product. It opens to its workspace, discovers
phones, explains authorization problems, and starts a mirror only when the user
chooses. Its engine and device services are included and managed automatically.
Session status must reflect what the phone and engine are doing. Recordings must
finish predictably, and connection failures must leave the app ready to recover.

Retain the approved interface, colors, seven navigation destinations and native
window controls. Develop the remaining work as independently demonstrable changes
to this baseline, each with behavioral acceptance criteria.

The product target now includes routes to all sixteen scrcpy documentation categories,
their applicable configuration options, and an Android Studio–style live-control toolbar.
Specialist camera, virtual-display, gamepad, OTG, tunnel and Linux webcam workflows
are in scope. Incremental preview releases may ship subsets with explicit coverage;
the original small configuration set is not the definition of feature completeness.
The current separate mirror window remains a development baseline, pending the
decision about in-workspace video for a complete release.
Bundling the engine and embedding the video are separate requirements: the former
is confirmed; the latter remains an explicit product/architecture decision.

## User Stories

1. As a desktop user, I want to install DroidCast as one product, so that I do not assemble a toolchain.
2. As a first-time user, I want a welcome screen before mirroring, so that I understand what to do next.
3. As a first-time user, I want USB debugging instructions, so that I can authorize my phone.
4. As a user, I want the approved interface and colors to remain familiar, so that improvements do not reset my workflow.
5. As a user, I want connected phones to appear automatically, so that I do not run terminal commands.
6. As a user with multiple connected phones, I want explicit selection, so that actions reach the intended device.
7. As a user, I want unauthorized, offline and permission-denied states distinguished, so that I know how to recover.
8. As a user, I want mirroring to start only when requested, so that connecting a phone does not expose its display unexpectedly.
9. As a user, I want starting, streaming, stopping and failed sessions distinguished, so that process launch is not mistaken for a working mirror.
10. As a user, I want a failed session to release its resources, so that I can try again without restarting DroidCast.
11. As a user, I want to reconnect a disconnected phone, so that a cable or network interruption does not end my workflow permanently.
12. As a user, I want a clear stop action, so that I can end mirroring safely.
13. As a user recording a session, I want stopping to finalize the file, so that it can be played afterward.
14. As a user, I want warnings about interrupted or incomplete recordings, so that I do not mistake an unfinished file for a successful capture.
15. As a user, I want wireless pairing and connection addresses explained separately, so that I enter the correct ports.
16. As a user, I want pairing failures and timeouts to be actionable, so that I can retry without guessing.
17. As a user, I want pairing codes kept out of saved preferences and logs, so that temporary credentials are not retained.
18. As a user, I want quality and FPS preferences saved, so that new sessions use my choices.
19. As a user, I want unsupported video/audio/input options explained, so that I can select compatible settings.
20. As a user, I want changes clearly identified as next-session settings, so that the app does not imply unsupported live changes.
21. As a user, I want screenshots of the chosen phone, so that I can capture exactly the device I selected.
22. As a user, I want captures saved to a chosen folder with unique names, so that earlier files remain intact.
23. As a user, I want a library of actual local captures, so that its contents match files on disk.
24. As a user, I want to filter and open captures, so that I can find and use them outside DroidCast.
25. As a user, I want recording-in-progress distinguished from finished media, so that I open files only when ready.
26. As a user, I want to install an APK on the selected phone, so that I can manage my own Android applications from the workspace.
27. As a user, I want installation errors shown honestly, so that a completed command is not mistaken for a successful install.
28. As a user, I want keyboard and mouse modes and shortcut guidance, so that I can control Android predictably.
29. As a keyboard user, I want visible focus and operable controls throughout the workspace, so that a mouse is optional.
30. As a user on a smaller or scaled display, I want content to remain readable and reachable, so that controls are not clipped.
31. As a user, I want local diagnostics with useful failure details, so that I can troubleshoot without exposing logs automatically.
32. As a user, I want DroidCast to run without network access after installation for USB use, so that runtime dependencies are not downloaded during startup.
33. As a Windows user, I want a tested Windows package, so that no compiler or development tools are required.
34. As a Linux user, I want a tested Linux package and clear USB permission guidance, so that the development machine is not a hidden dependency.
35. As a macOS user, I want a tested application bundle, so that runtime lookup does not depend on terminal environment variables.
36. As a maintainer, I want matching client/server builds and pinned dependencies, so that a release reproduces the intended fork.
37. As a maintainer, I want real-phone release checks, so that simulated test helpers are not the only evidence of functionality.
38. As a user, I want every scrcpy documentation category to have a discoverable route in DroidCast, so that supported capabilities do not require a terminal.
39. As a user, I want configuration search to understand plain terms and flag names, so that I can find an option from its documentation.
40. As a user, I want Home, Back, Recents, power and volume buttons beside my session, so that I can operate my phone without memorizing shortcuts.
41. As a user, I want phone rotation distinguished from mirror rotation, so that I change the intended thing.
42. As a user, I want notification, quick-settings and clipboard actions, so that routine Android interactions are accessible from the desktop.
43. As a user, I want live versus restart-required settings identified, so that controls describe their actual effect.
44. As a user, I want display/encoder discovery and advanced video/audio options, so that I can use the engine's capabilities without constructing commands.
45. As a user, I want gamepad and all supported keyboard/mouse modes, so that my peripherals work with compatible phone sessions.
46. As a user, I want a virtual display with app selection and resizing, so that an Android app can run in a desktop-oriented workspace.
47. As a user, I want a camera session with supported camera, size, torch and zoom controls, so that I can use my phone's camera stream.
48. As a Linux user, I want supported video routed to a virtual webcam, so that another desktop app can consume it.
49. As a user, I want USB OTG control without debugging authorization, so that a compatible phone can receive hardware-style input without ADB.
50. As a user, I want advanced connection routes for existing ADB servers and tunnels, so that I can access devices through my configured connection.
51. As a user, I want read-only mode enforced across every control, so that viewing a phone does not accidentally modify it.
52. As a user, I want unavailable options to explain platform or device limitations, so that visibility is not mistaken for universal support.
53. As a maintainer, I want a complete option/action coverage inventory, so that engine upgrades do not silently leave GUI features behind.

## Implementation Decisions

### Confirmed baseline

- Product name: DroidCast Desktop. Repository: nilsonpmjr/DroidCast.
- C++17 and Qt 6 desktop shell around the existing C client and Java Android server.
- Approved blue-gray visual system and AIStudio-inspired information architecture.
- Engine, ADB and Android server resolved relative to the application bundle, not through user-facing executable settings.
- Asynchronous device discovery and process orchestration; explicit session start.
- Local preferences and capture storage. No account or cloud backend is needed.
- One managed mirroring process at a time; multiple phones can still be discovered and selected.
- The separate mirror window is the implemented baseline, not an embedded live preview.
- The current build compiles the fork's C client, with a verified upstream matching server; it does not yet build the Java server as part of a release.

### Proposed changes for the complete workspace

- Extend the existing engine-controller boundary with explicit session events and state. UI pages consume that contract instead of interpreting raw process state as streaming readiness.
- Introduce the smallest fork-side lifecycle/control channel needed for reliable readiness and graceful stop. Its transport and portability are implementation decisions to validate, not already-selected technology.
- Keep operations associated with their target device and request until completion. A later selection must not redirect an in-flight operation.
- Report finalized, failed and interrupted recordings separately. An emergency kill remains a fallback and cannot silently mark a recording complete.
- Keep connection, session and device-operation failures recoverable without restarting the application or killing unrelated ADB sessions.
- Build and validate matching release components; deploy Qt plugins and native runtime dependencies alongside the application.
- Select package formats and supported OS/architecture versions based on verified clean-machine runs. Do not claim support merely because a platform archive is downloadable.
- Avoid a broad rewrite of the approved UI. Separate page responsibilities only as a concrete feature requires it and preserve observable behavior.
- Expand Settings with searchable category routes and advanced controls, keeping the approved top-level navigation recognizable.
- Expose existing Android control messages and renderer operations through a session-targeted desktop-to-engine interface. Global shortcut injection is not the toolbar architecture.
- Classify controls as launch-time, live, queries or application-managed policy; model incompatible session modes explicitly.
- Preserve read-only semantics across toolbar, file transfer, installation and clipboard routes.
- Cover every declared CLI option and documented live action; terminal-only flags receive explicit application policy rather than unnecessary GUI toggles.

## Testing Decisions

Proposed primary seam: the existing engine controller's public operations and emitted
events, exercised through representative workspace actions. The same acceptance
scenarios should run against a controllable test helper for deterministic failures
and the real engine plus a physical phone for release validation.

- Test user-observable outcomes: selected device, session state, recoverability, output files and saved preferences. Do not assert private methods, exact log phrasing or widget counts as the principal feature contract.
- Extend existing Qt Test coverage for discovery, argument boundaries, wireless failures, process timeouts, capture validation, installation results and synchronized settings.
- Use actual file readability and recording finalization as media acceptance criteria, not only exit codes or file existence.
- Exercise unauthorized phones, disconnect during startup, disconnect while recording, permission failures, disk-write failures, rapid repeat actions and application shutdown.
- Preserve focused UI checks for keyboard navigation, focus visibility, selection, scaling and all seven destinations; avoid broad brittle pixel snapshots.
- Validate package behavior in clean environments, from paths containing spaces, with empty PATH and without compilers, Qt SDKs or separately installed scrcpy/ADB.
- Run representative USB/wireless, input/audio and recording journeys on each supported platform with real Android hardware. Report coverage gaps rather than inferring success.
- Extend the same controller seam to live toolbar commands, including wrong-device prevention, unavailable modes, read-only enforcement and failure feedback.
- Validate configuration dependencies and round trips, including camera dimension conflicts, USB-only input modes, virtual-display cleanup and platform-specific outputs.
- Audit coverage against the actual fork CLI and documented actions. Configuration presence alone does not count as implemented functionality.

This test boundary is proposed for user review before ticket publication.

## Out of Scope

Proposed exclusions from this coverage effort, not permanent exclusions from the
product:

- Another interface redesign, an Electron/TypeScript rewrite, or replacement of the fork's mirroring engine.
- Cloud accounts, a hosted remote-access service, telemetry services and a backend. Existing ADB/tunnel endpoints remain in scope.
- iOS mirroring, root-dependent workflows and bypassing Android authorization.
- Concurrent live sessions, automatic reconnect, per-device profile libraries and custom game key mapping.
- Emulator-only sensor/location/battery/cellular simulation, snapshots and other facilities not provided by the physical-device scrcpy engine.
- Automatic application updates and an installer system that silently downloads engine components at first launch.
- A commitment to a particular embedded-video technique before evaluating the cross-platform approach; embedded-video release priority remains open.

## Further Notes

The existing Linux build and automated tests passed before the baseline commit.
The staged app was also launched outside the repository with an empty PATH.
No phone was connected for end-to-end validation. Windows/macOS builds, complete
Qt/native-library deployment, graceful recording shutdown and signed installers
have not been proven.

The newly added DroidCast images were not part of the baseline commit and are not
modified by this planning work. Their eventual product use should be decided when
reviewing release assets.

Signing identities, notarization access and physical test devices are release inputs
to identify early; they must not be treated as automatically available.

Scope correction: the user explicitly requested the full documented configuration
surface and emulator-style device controls after reviewing the first draft. The
earlier exclusion of specialist scrcpy workflows is superseded. Refer to the
companion feature inventory for category destinations, option coverage and live actions.
