#pragma once
#include "engine.h"
#include <QMainWindow>
#include <QTimer>
#include <functional>

class QAction;
class QComboBox;
class QLabel;
class QPushButton;
class QPlainTextEdit;
class QSystemTrayIcon;
class QSettings;
class QGroupBox;
class QMenu;

class LiveWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit LiveWindow(Engine *controller = nullptr, bool discover = true,
                      std::function<bool()> trayAvailable = {},
                      QSettings *uiPreferences = nullptr);
  ~LiveWindow() override;
  void startSession(const QString &recording = {});
  void stopSession();
  void fitWindow();
  QString selectedSerial() const;
  void setTrayEnabled(bool enabled);
  void setBackgroundEnabled(bool enabled);
  bool trayUsable() const;
  void refreshTrayAvailability();
  void restoreWindow();
  void requestQuit();
  void setUsbOffersEnabled(bool enabled);

signals:
  void shutdownReady();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  void updateDevices();
  void updateActions();
  void finishClose();
  void recordSession();
  void capture();
  void transfer();
  void showSettings();
  void processDeviceScan(bool success);
  void updateUsbOffer();
  void dismissUsbOffer();
  bool selectUsbOfferTarget();
  Engine *engine;
  QTimer trayWatch;
  bool closing = false, shutdownReadySent = false;
  QString lastSelectedSerial;
  bool trayEnabled = false, backgroundEnabled = false, lastTrayUsable = false;
  std::function<bool()> trayAvailable;
  QSettings *uiPreferences;
  QSystemTrayIcon *tray;
  QAction *trayState;
  bool usbOffersEnabled = true, usbBaselineReady = false;
  QHash<QString, Device> knownUsb;
  QHash<QString, quint64> usbGeneration;
  QStringList pendingUsbOffers;
  QString announcedUsb;
  QGroupBox *usbOffer;
  QLabel *usbOfferText;
  QPushButton *usbMirrorButton, *usbTransferButton;
  QAction *usbMirrorAction, *usbTransferAction, *usbDismissAction;
  QMenu *usbOfferMenu;
  QList<QAction *> presentationActions;
  QComboBox *devices;
  QLabel *deviceStatus, *sessionStatus, *feedback;
  QPushButton *startButton, *stopButton, *fitButton, *refreshButton;
  QAction *startAction, *stopAction, *fitAction, *recordAction, *captureAction,
      *transferAction, *cancelTransferAction;
  QPlainTextEdit *logs;
};
