#pragma once

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QTimer>
#include <QVariantMap>

struct SessionOption {
  QString key, category, title, help;
  QVariant initial;
  QStringList choices;
  int minimum = 0, maximum = 0;
};
const QList<SessionOption> &sessionOptions();
QVariant normalizedOption(const SessionOption &option, const QVariant &value);

struct Device {
  QString serial, state, model, connection;
  bool ready() const { return state == "device"; }
};

struct CameraCapability {
  QString id, facing, sensorSize, zoomRange;
  QStringList frameRates, sizes;
  QMap<QString, QStringList> highSpeedFrameRates;
};
QList<CameraCapability> parseCameraCapabilities(const QString &output);

struct DisplayCapability {
  int id;
  QString size;
};
struct VideoEncoderCapability {
  QString codec, name, attributes;
};
QList<DisplayCapability> parseDisplayCapabilities(const QString &output);
QList<VideoEncoderCapability>
parseVideoEncoderCapabilities(const QString &output);
struct AppCapability {
  QString name, package;
  bool system;
};
QList<AppCapability> parseAppCapabilities(const QString &output);

struct Preferences {
  QString adb = "adb", scrcpy = "scrcpy", server;
  int size = 1080, fps = 60, bitrate = 8;
  QString keyboard = "sdk", codec = "h264", mouse = "sdk", mediaDirectory;
  bool audio = true, awake = false, screenOff = false, top = false;
  QVariantMap options;
  static Preferences load(QSettings &settings);
  void save(QSettings &settings) const;
};

bool sessionOptionAvailable(const SessionOption &option,
                            const Preferences &prefs);
QString recordingFormat(const Preferences &prefs);
QString textOptionError(const QString &key, const QString &value);
struct PreferenceIssue {
  QString key, message;
};
PreferenceIssue sessionIssue(const Preferences &prefs, bool recording = false);
QList<Device> parseDevices(const QString &output);
QStringList mirrorArguments(const QString &serial, const Preferences &prefs,
                            const QString &recording = {});
bool validEndpoint(const QString &endpoint);
QString executablePath(const QString &program);
QString runtimeDirectory();
Preferences bundledPreferences();

class Engine : public QObject {
  Q_OBJECT
public:
  explicit Engine(QObject *parent = nullptr);
  ~Engine() override;
  Preferences preferences;
  QList<Device> devices;
  QString activeSerial, activeRecording;
  bool deviceBusy() const {
    return deviceTask.state() != QProcess::NotRunning || inspecting();
  }
  void capture(const QString &serial, const QString &path);
  void installApk(const QString &serial, const QString &path);
  enum class PhoneAction { Back, Home, Recents, Power, VolumeUp, VolumeDown };
  void phoneAction(const QString &serial, PhoneAction action);
  bool controlAllowed() const;
  bool usesAlternateDisplay() const {
    return running() && activeAlternateDisplay;
  }
  bool cameraSession() const { return running() && activeCamera; }
  bool captureAllowed(const QString &serial) const {
    return (!usesAlternateDisplay() && !cameraSession()) ||
           serial != activeSerial;
  }
  enum class WindowAction {
    Fullscreen,
    Fit,
    PixelPerfect,
    RotateLeft,
    RotateRight,
    Pause,
    Resume
  };
  void windowAction(WindowAction action);
  bool windowControlsAvailable() const;
  enum class CameraAction { TorchOn, TorchOff, ZoomIn, ZoomOut };
  void cameraAction(CameraAction action);
  bool cameraControlsAvailable() const;
  enum class AndroidAction {
    DisplayOff,
    DisplayOn,
    Notifications,
    QuickSettings,
    CollapsePanels,
    RotateDevice,
    ResetVideo
  };
  void androidAction(AndroidAction action);
  bool androidControlsAvailable() const;
  enum class SessionState {
    Idle,
    Starting,
    Streaming,
    Stopping,
    Ended,
    Failed,
    Disconnected
  };
  SessionState sessionState() const { return state; }
  QString sessionStateText() const;
  bool scanning() const { return scan.state() != QProcess::NotRunning; }
  bool running() const { return mirror.state() != QProcess::NotRunning; }
  bool wirelessBusy() const { return wireless.state() != QProcess::NotRunning; }
  void refresh();
  bool inspecting() const { return inspector.state() != QProcess::NotRunning; }
  void inspectDevice(const QString &serial);
  void cancelInspection();
  bool start(const QString &serial, const QString &recording = {});
  void stop();
  void connectWireless(const QString &endpoint, const QString &code = {});
signals:
  void devicesChanged();
  void sessionChanged();
  void scanChanged();
  void inspectionChanged();
  void inspectionResult(const QString &serial, const QString &output);
  void wirelessChanged();
  void deviceTaskChanged();
  void windowControlsChanged();
  void windowControlMessage(const QString &text);
  void cameraControlsChanged();
  void cameraControlMessage(const QString &text);
  void androidControlsChanged();
  void androidControlMessage(const QString &text);
  void captureSaved(const QString &path);
  void message(const QString &text);
  void log(const QString &text);

private:
  QProcess scan, mirror, wireless, deviceTask;
  QProcess inspector;
  QTimer inspectionTimeout;
  QByteArray inspectionOutput;
  QString inspectionSerial;
  bool inspectionAborted = false;
  QTimer scanTimeout, wirelessTimeout, stopTimeout, deviceTimeout;
  QByteArray deviceOutput;
  QString capturePath;
  bool commandTask = false, activeReadOnly = false;
  bool activeAlternateDisplay = false, activeFlexibleDisplay = false,
       activeCamera = false;
  bool deviceTimedOut = false;
  bool canUseDevice(const QString &serial);
  bool stopping = false, scanTimedOut = false, wirelessTimedOut = false;
  SessionState state = SessionState::Idle;
  bool forcedStop = false, bridgeReady = false, recordingFinalized = false,
       recordingFailed = false;
  QByteArray sessionToken, mirrorOutput;
  QTimer bridgeCommandTimeout;
  bool windowCommandsReady = false, windowCommandsHealthy = true;
  bool cameraCommandsReady = false, cameraCommandsHealthy = true;
  bool androidCommandsReady = false, androidCommandsHealthy = true;
  char pendingBridgeCommand = 0;
  void readMirrorOutput();
  QByteArray scanOutput, wirelessOutput, pairingInput;
};
