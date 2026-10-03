#pragma once

#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QTimer>

struct Device {
  QString serial, state, model, connection;
  bool ready() const { return state == "device"; }
};

struct Preferences {
  QString adb = "adb", scrcpy = "scrcpy", server;
  int size = 1080, fps = 60, bitrate = 8;
  QString keyboard = "sdk", codec = "h264", mouse = "sdk", mediaDirectory;
  bool audio = true, awake = false, screenOff = false, top = false;
  static Preferences load(QSettings &settings);
  void save(QSettings &settings) const;
};

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
  bool deviceBusy() const { return deviceTask.state() != QProcess::NotRunning; }
  void capture(const QString &serial, const QString &path);
  void installApk(const QString &serial, const QString &path);
  bool scanning() const { return scan.state() != QProcess::NotRunning; }
  bool running() const { return mirror.state() != QProcess::NotRunning; }
  bool wirelessBusy() const { return wireless.state() != QProcess::NotRunning; }
  void refresh();
  bool start(const QString &serial, const QString &recording = {});
  void stop();
  void connectWireless(const QString &endpoint, const QString &code = {});
signals:
  void devicesChanged();
  void sessionChanged();
  void scanChanged();
  void wirelessChanged();
  void deviceTaskChanged();
  void captureSaved(const QString &path);
  void message(const QString &text);
  void log(const QString &text);

private:
  QProcess scan, mirror, wireless, deviceTask;
  QTimer scanTimeout, wirelessTimeout, stopTimeout, deviceTimeout;
  QByteArray deviceOutput;
  QString capturePath;
  bool deviceTimedOut = false;
  bool canUseDevice(const QString &serial);
  bool stopping = false, scanTimedOut = false, wirelessTimedOut = false;
  QByteArray scanOutput, wirelessOutput, pairingInput;
};
