# DroidCast Desktop: proposed delivery tickets

Status: implementation authorized in incremental parts. These are not published GitHub issues.
Prepared using the installed `to-tickets` skill from the accompanying specification.
Numbers below are draft identifiers, not GitHub issue numbers.

Each ticket adds or proves a complete user-visible journey. Dependencies represent
actual gates; independent device workflows need not wait for all session work.

Scope expanded after user review: all documented scrcpy configuration families and
an Android Studio–style toolbar are part of the product target. Tickets 1–12 describe
the reliable core and an optional preview, not completion of that expanded target.
Tickets 13–26 add the complete configuration/control surface and its release gate.
The user authorized incremental implementation after reviewing the expanded scope.
Progress is recorded in `implementation-progress.md`; partial slices do not mark
these broader tickets complete.

1. **Show the actual session lifecycle**
   - **Blocked by:** None.
   - **What it delivers:** Starting a mirror displays starting, streaming, failure and ended states backed by engine events.
   - **Acceptance:** Launch alone never claims streaming; first-frame/readiness evidence drives the live state; failed startup clears the session; start/stop/retry works through the existing controller boundary.
   - **Verification:** Deterministic helper scenarios plus a USB phone journey; preserve the approved UI while adding state feedback.

2. **Stop a session and finish its recording cleanly**
   - **Blocked by:** 1, actual session lifecycle.
   - **What it delivers:** Stop and app-close finish a recording before releasing the session, with an explicit interrupted-file outcome on failure.
   - **Acceptance:** A normal stop produces playable media; app-close uses the same graceful path; a hung engine times out visibly; forced termination never reports finalized success; repeated stop is safe.
   - **Verification:** Real short recordings plus delayed, failed and unresponsive shutdown cases.

3. **Recover from unplugged and unavailable phones**
   - **Blocked by:** 1, actual session lifecycle.
   - **What it delivers:** Removing a phone or losing USB access gives a useful state and allows a new explicit connection/session without restarting DroidCast.
   - **Acceptance:** No stale ready devices after a failed scan; no orphan session after disconnect; selection is preserved when possible; reconnect does not auto-start mirroring; permission/authorization recovery is actionable.
   - **Verification:** Unplug during startup and streaming, reconnect, authorize and mirror again.

4. **Complete the wireless pair-to-mirror journey**
   - **Blocked by:** None.
   - **What it delivers:** Pair, connect, select and launch a wireless phone with retryable failures.
   - **Acceptance:** Separate pairing/connection ports remain clear; expired or wrong codes do not imply success; timeouts recover; secrets stay out of logs/preferences; failed connect with exit code zero remains a failure.
   - **Verification:** Deterministic failures and a real Android wireless pairing followed by mirroring; use existing launch behavior, adopting lifecycle events when ticket 1 lands.

5. **Make capture and APK actions reliable for the selected phone**
   - **Blocked by:** None.
   - **What it delivers:** Selecting one phone, capturing its screen or installing an APK, and inspecting the result works despite selection changes and failure conditions.
   - **Acceptance:** Target identity stays fixed per request; captures validate and save atomically; filesystem failures preserve earlier files; installation failure is visible; duplicate actions cannot collide; library entries reflect real files.
   - **Verification:** Two device identities, invalid PNG, write failure, install rejection and selection changes during an operation.

6. **Make session settings compatible and predictable**
   - **Blocked by:** None.
   - **What it delivers:** Users can choose video/audio/input settings, retain them across launches, and recover from unsupported choices with a clear explanation.
   - **Acceptance:** Quick and full settings remain synchronized; changes apply to the next session; unsupported options do not silently fall back; audio/input limitations are exposed when known; secrets and executable paths are not user preferences.
   - **Verification:** Saved settings round trip through launch arguments and a real session; unsupported-codec/audio/input scenarios.

7. **Build one complete matching runtime from the fork**
   - **Blocked by:** None.
   - **What it delivers:** A release build produces an app with a matching fork client/server, ADB, resources and an identifiable dependency manifest.
   - **Acceptance:** Server changes are reflected in the artifact; version mismatches fail visibly; verified cached dependencies support repeat builds; missing components fail packaging; execution does not depend on user PATH or executable configuration.
   - **Verification:** Build, stage, launch and discover a phone from outside the checkout; verify artifacts originate from the intended source revision.

8. **Ship a Linux package that works on a clean machine**
   - **Blocked by:** 7, complete matching runtime.
   - **What it delivers:** A Linux user installs/opens DroidCast and mirrors a phone without a development toolchain or separate scrcpy installation.
   - **Acceptance:** Include required Qt plugins/native libraries or explicitly supported system dependencies; check Wayland and X11 on supported environments; diagnose USB permissions; preserve captures/preferences on upgrade; provide distribution notices.
   - **Verification:** Clean-machine USB mirror, screenshot and short recording. Lifecycle refinements can land independently before the final release gate.

9. **Ship a Windows package that works on a clean machine**
   - **Blocked by:** 7, complete matching runtime.
   - **What it delivers:** A Windows user installs/opens DroidCast and mirrors a phone without Qt, compilers or separate Android tooling.
   - **Acceptance:** Deploy required DLLs/plugins; support paths with spaces/non-ASCII characters; avoid stray console windows; guide driver/authorization failures; preserve user data on upgrade; document signing status.
   - **Verification:** Clean-machine USB mirror, screenshot and short recording; validate supported Windows version/architecture explicitly.

10. **Ship a macOS app that works outside the development environment**
    - **Blocked by:** 7, complete matching runtime.
    - **What it delivers:** A macOS user opens the application bundle and mirrors a phone without terminal setup or Homebrew dependencies.
    - **Acceptance:** Deploy frameworks/plugins/native libraries; bundle lookup works from Finder; architecture support is tested; signing/notarization inputs are identified and integrated before public release; preserve user data on upgrade.
    - **Verification:** Clean-machine launch and USB capture journey on each claimed architecture; absence of signing credentials is reported as a release blocker, not silently bypassed.

11. **Make the approved workspace usable with keyboard and display scaling**
    - **Blocked by:** None.
    - **What it delivers:** All current pages work at supported desktop sizes/scales with keyboard focus, accessible names and clear selected-device feedback.
    - **Acceptance:** Controls do not clip or become unreachable; custom switches behave like accessible checkboxes; focus is visible; card selection is unambiguous; errors remain readable; the approved palette and navigation remain intact.
    - **Verification:** Representative small/scaled windows, keyboard-only journeys and accessibility inspection on platform packages as they become available.

12. **Validate a core cross-platform preview**
    - **Blocked by:** 2, 3, 4, 5, 6, 8, 9, 10 and 11. Tickets 1 and 7 are transitively required.
    - **What it delivers:** An optional versioned preview with reproducible artifacts, platform-specific evidence, checksums and accurate user instructions. It is not presented as full scrcpy feature coverage.
    - **Acceptance:** Each claimed platform passes the real-phone USB/wireless, input/audio, capture, graceful stop and recovery journeys; packages have no hidden SDK dependency; release notes state supported versions and remaining limitations; signing and distribution prerequisites are satisfied.
    - **Verification:** A recorded release checklist tied to artifact hashes and the source revision. Public publication is a separate explicit release action.

13. **Find and configure options through documented category routes**
    - **Blocked by:** None.
    - **What it delivers:** Searchable settings destinations for the sixteen documentation topics, with an initial complete video-resolution workflow proving search → edit → validate → save → launch.
    - **Acceptance:** Plain terms and flag names find the same control; basic/advanced groups preserve the approved layout; controls identify applicability and launch/live behavior; category coverage is tracked, and unfinished features are not fake working controls.
    - **Verification:** Navigate and search to resolution, persist it, launch and observe its effect. The option inventory has no unclassified engine flags.

14. **Control Android navigation, power and volume from the session toolbar**
    - **Blocked by:** 1, actual session lifecycle.
    - **What it delivers:** Accessible Back, Home, Recents, power and volume buttons target the selected active session through a real command interface.
    - **Acceptance:** Commands reach the intended phone without changing desktop keyboard focus; unavailable/read-only sessions disable them; failures are visible; repeated press/release does not leave stuck keys.
    - **Verification:** Controller-level command tests and a real-phone toolbar journey, including disconnect and read-only cases.

15. **Control phone display, panels, clipboard and app launch**
    - **Blocked by:** 14, toolbar command interface.
    - **What it delivers:** Screen on/off, Android rotation, notifications/quick settings, panel collapse, clipboard actions and app launch work from the session workspace.
    - **Acceptance:** Phone rotation is distinct from mirror rotation; screen power is not labelled device shutdown; clipboard policy and Android restrictions are honored; installed-app choices load asynchronously; no secrets are logged.
    - **Verification:** Exercise supported actions on a phone and assert read-only enforcement and wrong-target prevention through the controller seam.

16. **Control the mirror presentation from the workspace**
    - **Blocked by:** 14, toolbar command interface.
    - **What it delivers:** Fullscreen, fit, 1:1, image rotation/flips, display pause/resume and video reset control the actual mirror surface.
    - **Acceptance:** Presentation pause does not falsely imply paused Android or recording; actions work for a detached window and through the same abstraction for future embedding; window settings cover position/size, title, background, aspect lock, borders and rendering options.
    - **Verification:** Observe the actual surface changing; validate host actions separately from phone-mutating actions and test incompatible modes.

17. **Configure advanced display and video capture**
    - **Blocked by:** 13, category routes.
    - **What it delivers:** Select a physical display, codec/encoder, dimensions/FPS/bitrate, crop, capture/presentation orientation and buffering through the UI.
    - **Acceptance:** Encoder/display choices use real queries; advanced codec and compatibility controls validate their values; capture versus presentation geometry is clear; unsupported choices and restart requirements are visible.
    - **Verification:** Configurations round trip to a real session; test crop/orientation semantics, absent encoders and invalid combinations.

18. **Configure audio capture, playback and audio-only sessions**
    - **Blocked by:** 13, category routes.
    - **What it delivers:** Configure source, duplication, codec/encoder, bitrate and buffering, including audio-only and no-playback modes.
    - **Acceptance:** Android/source availability is explained; audio-only does not require an onscreen video preview; require-audio policy and mutually incompatible options are enforced; restricted sources are not promised to work everywhere.
    - **Verification:** Actual supported audio capture/playback plus source failures and no-audio paths; no success based solely on process launch.

19. **Use the full keyboard, mouse and gamepad mode set**
    - **Blocked by:** 13, category routes.
    - **What it delivers:** SDK/UHID/AOA/disabled modes where supported, text/key behavior, hover/repeat, mouse bindings, gamepad modes and shortcut modifier controls.
    - **Acceptance:** AOA is USB-only; OTG remains a separate mode; input conflicts validate before launch; gesture help describes SDK-mode limits; physical-keyboard settings and documented shortcuts are discoverable.
    - **Verification:** Real compatible peripherals and transport/mode validation; preserved host shortcuts and no stuck input on disconnect.

20. **Run an Android app on a virtual display**
    - **Blocked by:** 13, category routes.
    - **What it delivers:** Choose an app and virtual size/DPI, launch it, resize with flex display and select decoration/content-lifecycle/IME policies.
    - **Acceptance:** Empty displays without launcher/app have useful guidance; app discovery is real; resize is visible; closing applies the chosen content policy; virtual-display and camera/physical-display conflicts are rejected.
    - **Verification:** App launch, resize, keyboard entry and close on a supported phone, including a device without virtual-display launcher content.

21. **Use the phone camera with live torch and zoom controls**
    - **Blocked by:** 13, category routes; 14, toolbar command interface.
    - **What it delivers:** Choose camera ID/facing, size/aspect, FPS/high-speed and orientation, then control supported torch/zoom during capture.
    - **Acceptance:** Query actual camera capabilities; reject explicit-size versus automatic-size conflicts; expose unsupported Android/hardware states; camera-only controls do not leak into display sessions.
    - **Verification:** Real camera capture, camera selection and supported live controls; high-speed and unsupported-mode tests use capability-specific evidence.

22. **Control a phone over USB OTG without ADB**
    - **Blocked by:** 13, category routes.
    - **What it delivers:** Discover/select an eligible USB device and start an OTG keyboard/mouse/gamepad session without requiring USB debugging.
    - **Acceptance:** Do not use the ADB-authorized-device gate; label the mode as control-only with no audio/video; expose transport/driver restrictions; stop and reconnect recover cleanly.
    - **Verification:** Compatible USB hardware with ADB authorization absent, plus Windows driver failure and unsupported transport behavior.

23. **Connect through an existing remote ADB endpoint or tunnel**
    - **Blocked by:** 13, category routes.
    - **What it delivers:** Configure a remote ADB socket and tunnel host/port/forward mode, discover a phone through it and start a session.
    - **Acceptance:** Settings apply consistently to discovery, capabilities and all device actions; normal local configuration remains recoverable; no automatic public ADB exposure, shared-server termination or implicit SSH credential management.
    - **Verification:** A controlled existing tunnel from discovery through mirroring, unreachable endpoints and switching back to local operation.

24. **Publish display video to a Linux virtual webcam**
    - **Blocked by:** 13, category routes; 17, video source settings.
    - **What it delivers:** Select an existing V4L2 output and buffering, start a display source and consume it from another desktop application.
    - **Acceptance:** Enable and package the engine capability currently disabled in the desktop build; detect device/prerequisite issues; mark Linux-only availability; do not install kernel modules without explicit authority; accept camera sources when camera support lands.
    - **Verification:** Read real frames from the selected virtual device in another application, plus unavailable-build/device/platform cases.

25. **Configure advanced recording and non-mirroring workflows**
    - **Blocked by:** 2, graceful finalization; 13, category routes; 18, audio modes.
    - **What it delivers:** Choose compatible containers, recording orientation, time limits, audio-only recording, and recording with selected playback disabled.
    - **Acceptance:** Validate stream/codec/container combinations; keep capture filenames unique; no-window affects the engine display rather than hiding the management app; runtime record/pause controls are not advertised until genuinely supported.
    - **Verification:** Play back finalized audio/video files and observe time-limit/stop behavior for representative supported combinations.

26. **Verify complete configuration and toolbar coverage**
    - **Blocked by:** 2–6, 8–11 and 13–25. Dependencies 1 and 7 are transitively required; optional preview ticket 12 is not a gate.
    - **What it delivers:** A complete desktop release whose documented features are reachable, functional where supported, and accurately constrained elsewhere.
    - **Acceptance:** Every option and action has a control/query/managed-policy classification; all sixteen documentation routes resolve; engine-version changes trigger a coverage audit; read-only applies to every mutating path; platform packages pass their applicable real-device journeys.
    - **Verification:** The feature matrix has no unclassified flags or unverified claimed features. Unsupported platform/device features have accurate explanations rather than fake success. Resolve the in-workspace-video release decision before freezing this gate.

## Recommended order

Start with **1, actual session lifecycle**, and **13, searchable category routes**.
Then build **14, daily toolbar controls**, **2, graceful stop**, and **3, recovery**.
Bring standard video/audio/input configuration into the daily workflow before the
specialist virtual-display/camera/OTG/tunnel/output modes. Runtime and platform
packaging proceed along their own dependencies. Ticket 12 is an optional core preview;
ticket 26 is the expanded feature-completeness gate.

## Live video inside the workspace

This is a separate decision before freezing the first-release scope. The app currently
bundles and launches its engine, but does not render live video inside the Qt page.
If embedded video is required for the first release, add a small end-to-end prototype
first: show a real phone frame in the workspace using a portable rendering boundary,
then validate input, resize, clipboard, audio and recording incrementally. Its results
must determine the implementation tickets; do not promise a cross-platform solution
based on Linux-only window reparenting.

## Review requested before publishing

- Does the engine-controller boundary, with UI journeys and real-phone release checks, match the expected testing approach?
- Is this staged route to full configuration/control coverage correct, and is embedded video required before the complete release?
- Is each ticket small enough to review and demonstrate? Should any be split or combined?
- Are any listed dependencies unnecessary, or are any real gates missing?
- Use GitHub Issues in nilsonpmjr/DroidCast with `ready-for-agent`, or keep the tracker local?

After approval, publish each implementation ticket separately, link the spec and
wire native issue dependencies where supported. The parent specification is not
closed or modified by ticket creation. No implementation begins merely because
these drafts exist.
