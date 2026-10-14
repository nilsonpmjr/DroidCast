#pragma once
#include <QMainWindow>

class QAction;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSystemTrayIcon;
class QGroupBox;

class PrototypeWindow : public QMainWindow {
public:
  explicit PrototypeWindow(bool noTray = false);
  void simulateConnection(bool authorized = true);
  void simulateDisconnect();
  void simulateAuthorization();
  void startDemo();
  void stopDemo();
  void setTrayEnabled(bool enabled);
  bool sessionActive() const { return active; }
  bool offerVisible() const;
  bool trayUsable() const;

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  void updateState();
  void showSettings();
  void transferDemo();
  void dismissOffer();
  void restoreWindow();
  bool connected = false, authorized = false, active = false;
  bool background = false, offersEnabled = true, trayEnabled = false;
  bool trayForcedUnavailable = false;
  QString activeMode;
  QSystemTrayIcon *tray;
  QGroupBox *offer;
  QLabel *deviceStatus, *sessionStatus, *offerStatus;
  QComboBox *deviceChoice, *mode;
  QAction *startAction, *stopAction, *transferAction, *trayAction;
  QPushButton *startButton, *stopButton, *offerMirror, *offerTransfer;
  QCheckBox *suppressOffers;
};
