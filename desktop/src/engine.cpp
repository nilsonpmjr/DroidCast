#include "engine.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

const QList<SessionOption> &sessionOptions() {
  static const QList<SessionOption> options{
      {"video-buffer",
       "Video",
       "Video buffer (ms)",
       "More buffering can smooth playback at the cost of latency.",
       0,
       {},
       0,
       2000},
      {"display-orientation",
       "Video",
       "Display orientation",
       "Rotate the computer display, not the Android device.",
       "0",
       {"0", "90", "180", "270", "flip0", "flip90", "flip180", "flip270"}},
      {"audio-codec",
       "Audio",
       "Audio codec",
       "Requires a compatible encoder on the phone.",
       "opus",
       {"opus", "aac", "flac", "raw"}},
      {"audio-source",
       "Audio",
       "Audio source",
       "Output requires Android 11+. Playback requires Android 13+; apps may "
       "opt out. Microphone captures the phone microphone.",
       "output",
       {"output", "playback", "mic"}},
      {"audio-buffer",
       "Audio",
       "Audio buffer (ms)",
       "Leave at zero to use the engine default for the selected codec.",
       0,
       {},
       0,
       2000},
      {"audio-output-buffer",
       "Audio",
       "Output buffer (ms)",
       "Computer audio output buffering. Smaller values may cause glitches.",
       10,
       {},
       1,
       1000},
      {"no-control",
       "Device",
       "Read-only session",
       "Disable Android input and APK installation. Screenshots remain "
       "available.",
       false,
       {}},
      {"show-touches",
       "Device",
       "Show physical touches",
       "Shows touches made on the phone, not clicks injected from the "
       "computer.",
       false,
       {}},
      {"power-off-on-close",
       "Device",
       "Turn screen off on close",
       "Turns the display off when the session ends; does not shut down "
       "Android.",
       false,
       {}},
      {"no-clipboard-autosync",
       "Device",
       "Disable clipboard synchronization",
       "Prevent automatic clipboard sharing with the phone.",
       false,
       {}},
      {"fullscreen",
       "Window",
       "Start fullscreen",
       "Open the next mirror window fullscreen.",
       false,
       {}},
      {"window-borderless",
       "Window",
       "Borderless mirror",
       "Remove the mirror window decorations.",
       false,
       {}},
      {"disable-screensaver",
       "Window",
       "Keep computer awake",
       "Disable the computer screensaver while mirroring.",
       false,
       {}}};
  return options;
}

QVariant normalizedOption(const SessionOption &option, const QVariant &value) {
  if (!value.isValid())
    return option.initial;
  if (!option.choices.isEmpty())
    return option.choices.contains(value.toString()) ? value : option.initial;
  if (option.initial.metaType().id() == QMetaType::Bool)
    return value.toBool();
  bool ok = false;
  int number = value.toInt(&ok);
  return ok ? QVariant(qBound(option.minimum, number, option.maximum))
            : option.initial;
}

Preferences Preferences::load(QSettings &s) {
  Preferences p = bundledPreferences();
  p.size = qBound(0, s.value("session/size", p.size).toInt(), 8192);
  p.fps = qBound(1, s.value("session/fps", p.fps).toInt(), 240);
  p.bitrate = qBound(1, s.value("session/bitrate", p.bitrate).toInt(), 200);
  p.keyboard = s.value("session/keyboard", p.keyboard).toString();
  if (p.keyboard != "sdk" && p.keyboard != "uhid")
    p.keyboard = "sdk";
  p.mouse = s.value("session/mouse", p.mouse).toString();
  if (p.mouse != "sdk" && p.mouse != "uhid")
    p.mouse = "sdk";
  p.codec = s.value("session/codec", p.codec).toString();
  if (!QStringList{"h264", "h265", "av1"}.contains(p.codec))
    p.codec = "h264";
  p.mediaDirectory = s.value("captures/directory", p.mediaDirectory).toString();
  p.audio = s.value("session/audio", p.audio).toBool();
  p.awake = s.value("session/awake", p.awake).toBool();
  p.screenOff = s.value("session/screenOff", p.screenOff).toBool();
  p.top = s.value("session/top", p.top).toBool();
  for (const auto &option : sessionOptions())
    p.options.insert(
        option.key, normalizedOption(option, s.value("options/" + option.key)));
  return p;
}

void Preferences::save(QSettings &s) const {
  s.setValue("session/codec", codec);
  s.setValue("session/mouse", mouse);
  s.setValue("captures/directory", mediaDirectory);
  s.setValue("session/size", size);
  s.setValue("session/fps", fps);
  s.setValue("session/bitrate", bitrate);
  s.setValue("session/keyboard", keyboard);
  s.setValue("session/audio", audio);
  s.setValue("session/awake", awake);
  s.setValue("session/screenOff", screenOff);
  s.setValue("session/top", top);
  for (const auto &option : sessionOptions())
    s.setValue("options/" + option.key,
               normalizedOption(option, options.value(option.key)));
}

QList<Device> parseDevices(const QString &output) {
  QList<Device> devices;
  for (const auto &line : output.split('\n')) {
    const auto fields =
        line.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (fields.size() < 2 || line.startsWith("List ") || line.startsWith('*'))
      continue;
    const auto state = fields[1];
    if (state != "device" && state != "offline" && state != "unauthorized" &&
        state != "no" && state != "recovery" && state != "sideload" &&
        state != "bootloader")
      continue;
    Device d{fields[0], state == "no" ? "no permissions" : state, fields[0],
             "USB"};
    if (d.serial.contains(':') || d.serial.contains("_adb-tls-connect"))
      d.connection = "Wireless";
    else if (d.serial.startsWith("emulator-"))
      d.connection = "Emulator";
    for (const auto &field : fields) {
      if (field.startsWith("model:"))
        d.model = field.mid(6).replace('_', ' ');
    }
    devices.append(d);
  }
  return devices;
}

QStringList mirrorArguments(const QString &serial, const Preferences &p,
                            const QString &recording) {
  QStringList args{"--serial=" + serial,
                   "--max-fps=" + QString::number(p.fps),
                   "--video-bit-rate=" + QString::number(p.bitrate) + "M",
                   "--keyboard=" + p.keyboard,
                   "--mouse=" + p.mouse,
                   "--video-codec=" + p.codec,
                   "--window-title=DroidCast Desktop | " + serial};
  if (p.size > 0)
    args << "--max-size=" + QString::number(p.size);
  if (!p.audio)
    args << "--no-audio";
  if (p.awake)
    args << "--stay-awake";
  if (p.screenOff)
    args << "--turn-screen-off";
  if (p.top)
    args << "--always-on-top";
  if (!recording.isEmpty())
    args << "--record=" + recording;
  const bool readOnly = p.options.value("no-control", false).toBool();
  if (readOnly) {
    args.removeAll("--stay-awake");
    args.removeAll("--turn-screen-off");
  }
  for (const auto &option : sessionOptions()) {
    const auto value = normalizedOption(option, p.options.value(option.key));
    if (value == option.initial)
      continue;
    if (!p.audio && option.category == "Audio")
      continue;
    if (readOnly &&
        (option.key == "show-touches" || option.key == "power-off-on-close"))
      continue;
    if (option.initial.metaType().id() == QMetaType::Bool) {
      if (value.toBool())
        args << "--" + option.key;
    } else
      args << "--" + option.key + "=" + value.toString();
  }
  return args;
}

bool validEndpoint(const QString &endpoint) {
  // Require an explicit port: Android's pairing and connection ports differ.
  static const QRegularExpression pattern(
      "\\A(?:[A-Za-z0-9](?:[A-Za-z0-9.-]*[A-Za-z0-9])?|\\[[0-9A-Fa-f:]+\\]):(["
      "0-9]{1,5})\\z");
  auto match = pattern.match(endpoint);
  return match.hasMatch() && match.captured(1).toInt() > 0 &&
         match.captured(1).toInt() <= 65535;
}

QString executablePath(const QString &program) {
  if (program.isEmpty())
    return {};
  return QStandardPaths::findExecutable(program);
}

QString runtimeDirectory() {
  // Build/test override only; never exposed as an end-user setting.
  return qEnvironmentVariable("DROIDCAST_RUNTIME_DIR",
                              QCoreApplication::applicationDirPath() +
                                  "/runtime");
}

Preferences bundledPreferences() {
  Preferences p;
  const QDir runtime(runtimeDirectory());
#ifdef Q_OS_WIN
  p.adb = runtime.filePath("adb.exe");
  p.scrcpy = runtime.filePath("scrcpy.exe");
#else
  p.adb = runtime.filePath("adb");
  p.scrcpy = runtime.filePath("scrcpy");
#endif
  p.server = runtime.filePath("scrcpy-server");
  p.mediaDirectory =
      QDir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))
          .filePath("DroidCast");
  return p;
}

Engine::Engine(QObject *parent) : QObject(parent) {
  scanTimeout.setSingleShot(true);
  wirelessTimeout.setSingleShot(true);
  stopTimeout.setSingleShot(true);
  connect(&scanTimeout, &QTimer::timeout, this, [this] {
    scanTimedOut = true;
    scan.kill();
    emit message("Device scan timed out. Check ADB and retry.");
  });
  connect(&wirelessTimeout, &QTimer::timeout, this, [this] {
    wirelessTimedOut = true;
    wireless.kill();
    emit message(
        "Wireless request timed out. Check the address and phone, then retry.");
  });
  connect(&stopTimeout, &QTimer::timeout, this, [this] {
    if (running()) {
      forcedStop = true;
      emit message("scrcpy did not exit; forcing it to stop. A recording may "
                   "be incomplete.");
      mirror.kill();
    }
  });
  connect(&scan, &QProcess::stateChanged, this, [this] { emit scanChanged(); });
  connect(&scan, &QProcess::readyReadStandardOutput, this,
          [this] { scanOutput += scan.readAllStandardOutput(); });
  connect(&scan, &QProcess::readyReadStandardError, this,
          [this] { emit log(QString::fromUtf8(scan.readAllStandardError())); });
  connect(&scan, &QProcess::errorOccurred, this,
          [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
              scanTimeout.stop();
              devices.clear();
              emit devicesChanged();
              emit message("The bundled connection service could not start. "
                           "See Diagnostics or reinstall DroidCast Desktop.");
            }
          });
  connect(&scan, &QProcess::finished, this,
          [this](int code, QProcess::ExitStatus status) {
            scanTimeout.stop();
            scanOutput += scan.readAllStandardOutput();
            devices =
                code == 0 && status == QProcess::NormalExit && !scanTimedOut
                    ? parseDevices(QString::fromUtf8(scanOutput))
                    : QList<Device>{};
            emit devicesChanged();
            if (!scanTimedOut && (code != 0 || status != QProcess::NormalExit))
              emit message(
                  "ADB scan failed. Open Diagnostics for details, then retry.");
          });
  connect(&mirror, &QProcess::readyReadStandardOutput, this,
          &Engine::readMirrorOutput);
  connect(&mirror, &QProcess::readyReadStandardError, this, [this] {
    emit log(QString::fromUtf8(mirror.readAllStandardError()));
  });
  connect(&mirror, &QProcess::started, this, [this] {
    if (!stopping)
      emit message(
          "Connecting to the phone. Waiting for the first video frame…");
    emit sessionChanged();
  });
  connect(&mirror, &QProcess::errorOccurred, this,
          [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
              stopTimeout.stop();
              state = SessionState::Failed;
              stopping = false;
              activeSerial.clear();
              activeRecording.clear();
              emit message("The bundled mirror engine could not start. See "
                           "Diagnostics or reinstall DroidCast Desktop.");
              emit sessionChanged();
            }
          });
  connect(&mirror, &QProcess::finished, this,
          [this](int code, QProcess::ExitStatus status) {
            stopTimeout.stop();
            readMirrorOutput();
            const bool clean = code == 0 && status == QProcess::NormalExit &&
                               !forcedStop && !recordingFailed;
            const bool recording = !activeRecording.isEmpty();
            if (!clean && state != SessionState::Disconnected)
              state = SessionState::Failed;
            else if (state != SessionState::Disconnected &&
                     state != SessionState::Failed)
              state = SessionState::Ended;
            activeSerial.clear();
            activeRecording.clear();
            if (forcedStop)
              emit message("The engine was forced to stop. Any recording may "
                           "be incomplete.");
            else if (recording && (!recordingFinalized || recordingFailed))
              emit message("Recording finalization was not confirmed. The file "
                           "may be incomplete; see Diagnostics.");
            else if (state == SessionState::Disconnected)
              emit message(
                  "Phone disconnected. Reconnect it and start a new session.");
            else if (state == SessionState::Failed)
              emit message(
                  "scrcpy exited with an error. Open Diagnostics for details.");
            else
              emit message(
                  recording ? "Recording finalized. Session ended."
                            : "Session ended. You can start another mirror.");
            stopping = false;
            emit sessionChanged();
          });
  wireless.setProcessChannelMode(QProcess::MergedChannels);
  connect(&wireless, &QProcess::stateChanged, this,
          [this] { emit wirelessChanged(); });
  connect(&wireless, &QProcess::started, this, [this] {
    if (!pairingInput.isEmpty()) {
      wireless.write(pairingInput);
      wireless.closeWriteChannel();
      pairingInput.fill('\0');
      pairingInput.clear();
    }
  });
  connect(&wireless, &QProcess::readyReadStandardOutput, this,
          [this] { wirelessOutput += wireless.readAllStandardOutput(); });
  connect(&wireless, &QProcess::errorOccurred, this,
          [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
              wirelessTimeout.stop();
              pairingInput.fill('\0');
              pairingInput.clear();
              emit message("The bundled connection service could not start. "
                           "See Diagnostics for details.");
            }
          });
  connect(&wireless, &QProcess::finished, this,
          [this](int code, QProcess::ExitStatus status) {
            wirelessTimeout.stop();
            wirelessOutput += wireless.readAllStandardOutput();
            auto result = QString::fromUtf8(wirelessOutput).trimmed();
            emit log(result);
            // ADB may return exit code zero even for a failed connect.
            bool success =
                code == 0 && status == QProcess::NormalExit &&
                (result.contains("connected to", Qt::CaseInsensitive) ||
                 result.contains("Successfully paired", Qt::CaseInsensitive));
            if (!wirelessTimedOut)
              emit message(success ? result
                                   : "Wireless request failed. " + result);
            if (success)
              refresh();
          });
  deviceTimeout.setSingleShot(true);
  connect(&deviceTimeout, &QTimer::timeout, this, [this] {
    deviceTimedOut = true;
    deviceTask.kill();
    emit message(
        "The device operation timed out. Reconnect your phone and try again.");
  });
  connect(&deviceTask, &QProcess::stateChanged, this,
          [this] { emit deviceTaskChanged(); });
  connect(&deviceTask, &QProcess::readyReadStandardOutput, this, [this] {
    deviceOutput += deviceTask.readAllStandardOutput();
    if (deviceOutput.size() > 32 * 1024 * 1024) {
      deviceTimedOut = true;
      deviceTask.kill();
      emit message(
          "The device returned too much data. The capture was not saved.");
    }
  });
  connect(&deviceTask, &QProcess::readyReadStandardError, this, [this] {
    emit log(QString::fromUtf8(deviceTask.readAllStandardError()));
  });
  connect(
      &deviceTask, &QProcess::errorOccurred, this,
      [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
          deviceTimeout.stop();
          emit message(
              "The bundled device service could not start. See Diagnostics.");
        }
      });
  connect(&deviceTask, &QProcess::finished, this,
          [this](int code, QProcess::ExitStatus status) {
            deviceTimeout.stop();
            deviceOutput += deviceTask.readAllStandardOutput();
            if (deviceTimedOut)
              return;
            if (code != 0 || status != QProcess::NormalExit) {
              emit message(
                  "Device operation failed. See Diagnostics for details.");
              return;
            }
            if (commandTask) {
              emit log(QString::fromUtf8(deviceOutput).trimmed());
              emit message("Phone command completed.");
            } else if (capturePath.isEmpty()) {
              const auto result = QString::fromUtf8(deviceOutput).trimmed();
              emit log(result);
              emit message(result.contains("Success")
                               ? "Application installed on your phone."
                               : "Installation failed. " + result);
            } else {
              if (QImage::fromData(deviceOutput, "PNG").isNull()) {
                emit message("The phone did not return a valid screenshot. "
                             "Nothing was saved.");
                return;
              }
              QSaveFile file(capturePath);
              if (!file.open(QIODevice::WriteOnly) ||
                  file.write(deviceOutput) != deviceOutput.size() ||
                  !file.commit()) {
                emit message("Could not save the screenshot. Check folder "
                             "permissions and available space.");
                return;
              }
              emit message("Screenshot saved to your capture library.");
              emit captureSaved(capturePath);
            }
          });
}

Engine::~Engine() {
  for (auto *process : {&scan, &wireless, &mirror, &deviceTask}) {
    if (process->state() != QProcess::NotRunning) {
      if (process == &mirror)
        process->write("Q", 1);
      else
        process->terminate();
      if (!process->waitForFinished(1500)) {
        process->kill();
        process->waitForFinished(1500);
      }
    }
  }
}

void Engine::refresh() {
  if (scanning())
    return;
  if (executablePath(preferences.adb).isEmpty()) {
    devices.clear();
    emit devicesChanged();
    emit message("DroidCast's bundled connection service is missing. Reinstall "
                 "the complete app package.");
    return;
  }
  scanOutput.clear();
  scanTimedOut = false;
  scanTimeout.start(10000);
  scan.start(preferences.adb, {"devices", "-l"});
}

bool Engine::start(const QString &serial, const QString &recording) {
  if (running())
    return false;
  bool authorized = false;
  for (const auto &device : devices)
    if (device.serial == serial && device.ready())
      authorized = true;
  if (!authorized) {
    emit message("Select an authorized device before starting a mirror.");
    return false;
  }
  if (executablePath(preferences.scrcpy).isEmpty() ||
      executablePath(preferences.adb).isEmpty()) {
    emit message("DroidCast's engine bundle is incomplete. Reinstall the "
                 "complete app package.");
    return false;
  }
  if (!preferences.server.isEmpty() &&
      !QFileInfo(preferences.server).isFile()) {
    emit message("DroidCast's Android server is missing. Reinstall the "
                 "complete app package.");
    return false;
  }
  auto env = QProcessEnvironment::systemEnvironment();
  env.insert("ADB", executablePath(preferences.adb));
  if (!preferences.server.isEmpty())
    env.insert("SCRCPY_SERVER_PATH", preferences.server);
  activeSerial = serial;
  activeRecording = recording;
  activeReadOnly = preferences.options.value("no-control", false).toBool();
  stopping = false;
  state = SessionState::Starting;
  forcedStop = bridgeReady = recordingFinalized = recordingFailed = false;
  mirrorOutput.clear();
  sessionToken = QUuid::createUuid().toString(QUuid::Id128).toLatin1();
  env.insert("DROIDCAST_SESSION_TOKEN", QString::fromLatin1(sessionToken));
  mirror.setProcessEnvironment(env);
  const auto args = mirrorArguments(serial, preferences, recording);
  emit log("Starting " + preferences.scrcpy +
           "\nArguments: " + args.join(" | "));
  mirror.start(preferences.scrcpy, args);
  emit sessionChanged();
  return true;
}

void Engine::stop() {
  if (!running() || stopping)
    return;
  stopping = true;
  state = SessionState::Stopping;
  emit message("Stopping session and finishing any recording…");
  // The fork handles this on its SDL event loop, following window-close
  // cleanup. QProcess buffers the byte if the process is still starting.
  mirror.write("Q", 1);
  stopTimeout.start(5000);
  emit sessionChanged();
}

QString Engine::sessionStateText() const {
  switch (state) {
  case SessionState::Idle:
    return "No active session";
  case SessionState::Starting:
    return "Starting session";
  case SessionState::Streaming:
    return "Streaming";
  case SessionState::Stopping:
    return "Finishing session";
  case SessionState::Ended:
    return "Session ended";
  case SessionState::Failed:
    return "Session failed";
  case SessionState::Disconnected:
    return "Phone disconnected";
  }
  return {};
}

void Engine::readMirrorOutput() {
  mirrorOutput += mirror.readAllStandardOutput();
  const QByteArray prefix = "DROIDCAST/1 " + sessionToken + ' ';
  qsizetype newline;
  while ((newline = mirrorOutput.indexOf('\n')) >= 0) {
    const auto line = mirrorOutput.left(newline).trimmed();
    mirrorOutput.remove(0, newline + 1);
    if (!line.startsWith(prefix)) {
      emit log(QString::fromUtf8(line));
      continue;
    }
    const auto event = line.mid(prefix.size());
    if (event == "bridge-ready")
      bridgeReady = true;
    else if (event == "first-frame" && bridgeReady &&
             state == SessionState::Starting) {
      state = SessionState::Streaming;
      emit message("Phone video is streaming in the mirror window.");
    } else if (event == "recording-finalized" && bridgeReady)
      recordingFinalized = true;
    else if (event == "recording-error" && bridgeReady)
      recordingFailed = true;
    else if (event == "disconnected" && bridgeReady)
      state = SessionState::Disconnected;
    else if (event == "failed" && bridgeReady)
      state = SessionState::Failed;
    emit sessionChanged();
  }
  if (mirrorOutput.size() > 65536) {
    emit log(QString::fromUtf8(mirrorOutput.left(65536)));
    mirrorOutput.clear();
  }
}

void Engine::connectWireless(const QString &endpoint, const QString &code) {
  if (wirelessBusy())
    return;
  if (!validEndpoint(endpoint) ||
      (!code.isEmpty() &&
       !QRegularExpression("\\A[0-9]{6}\\z").match(code).hasMatch())) {
    emit message("Enter a host:port address and, for pairing, the six-digit "
                 "code shown on the phone.");
    return;
  }
  wirelessOutput.clear();
  wirelessTimedOut = false;
  pairingInput = code.isEmpty() ? QByteArray{} : code.toUtf8() + '\n';
  wirelessTimeout.start(30000);
  emit message(code.isEmpty() ? "Connecting to phone…" : "Pairing with phone…");
  wireless.start(preferences.adb,
                 {code.isEmpty() ? "connect" : "pair", endpoint});
}

bool Engine::canUseDevice(const QString &serial) {
  if (deviceBusy())
    return false;
  for (const auto &d : devices)
    if (d.serial == serial && d.ready())
      return true;
  emit message("Choose an authorized device first.");
  return false;
}

void Engine::capture(const QString &serial, const QString &path) {
  if (!canUseDevice(serial) || path.isEmpty())
    return;
  capturePath = path;
  commandTask = false;
  deviceOutput.clear();
  deviceTimedOut = false;
  deviceTimeout.start(20000);
  emit message("Capturing phone screen…");
  deviceTask.start(preferences.adb,
                   {"-s", serial, "exec-out", "screencap", "-p"});
}

void Engine::installApk(const QString &serial, const QString &path) {
  if (!controlAllowed()) {
    emit message(
        "APK installation is disabled while stopping or in read-only mode.");
    return;
  }
  if (running() && serial != activeSerial) {
    emit message("APK installation must target the active session.");
    return;
  }
  if (!canUseDevice(serial))
    return;
  if (!QFileInfo(path).isFile() ||
      !path.endsWith(".apk", Qt::CaseInsensitive)) {
    emit message("Choose an existing Android APK file.");
    return;
  }
  capturePath.clear();
  commandTask = false;
  deviceOutput.clear();
  deviceTimedOut = false;
  deviceTimeout.start(120000);
  emit message("Installing application on your phone…");
  deviceTask.start(preferences.adb, {"-s", serial, "install", path});
}

bool Engine::controlAllowed() const {
  return !stopping &&
         !(running() ? activeReadOnly
                     : preferences.options.value("no-control", false).toBool());
}

void Engine::phoneAction(const QString &serial, PhoneAction action) {
  if (!controlAllowed()) {
    emit message(
        "Phone controls are disabled while stopping or in read-only mode.");
    return;
  }
  if (running() && serial != activeSerial) {
    emit message("Phone controls must target the active session.");
    return;
  }
  if (!canUseDevice(serial))
    return;
  QString key;
  switch (action) {
  case PhoneAction::Back:
    key = "KEYCODE_BACK";
    break;
  case PhoneAction::Home:
    key = "KEYCODE_HOME";
    break;
  case PhoneAction::Recents:
    key = "KEYCODE_APP_SWITCH";
    break;
  case PhoneAction::Power:
    key = "KEYCODE_POWER";
    break;
  case PhoneAction::VolumeUp:
    key = "KEYCODE_VOLUME_UP";
    break;
  case PhoneAction::VolumeDown:
    key = "KEYCODE_VOLUME_DOWN";
    break;
  default:
    return;
  }
  commandTask = true;
  capturePath.clear();
  deviceOutput.clear();
  deviceTimedOut = false;
  deviceTimeout.start(10000);
  deviceTask.start(preferences.adb,
                   {"-s", serial, "shell", "input", "keyevent", key});
}
