#pragma once
#include "engine.h"
#include <QElapsedTimer>
#include <QMainWindow>
class QListWidget;
class QStackedWidget;
class QLabel;
class QPushButton;
class QPlainTextEdit;
class QCloseEvent;
class QComboBox;
class Window : public QMainWindow {
  Q_OBJECT
public:
  explicit Window(QWidget *parent = nullptr);
  void showPage(int index);

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  QSettings settings;
  Engine engine;
  QTimer poll, sessionClock;
  QElapsedTimer elapsed;
  QListWidget *navigation, *deviceList, *captureList;
  QStackedWidget *pages;
  QLabel *notice, *deviceHelp, *sessionStatus, *runtimeStatus, *deviceCount,
      *sessionDetail, *captureDirectory;
  QLabel *headerStatus, *sessionTime;
  QWidget *welcome;
  QPushButton *startButton, *recordButton, *stopButton, *refreshButton,
      *screenshotButton, *apkButton, *sessionStop;
  QPlainTextEdit *logs;
  QComboBox *captureFilter;
  QString fingerprint;
  bool closing = false;
  QWidget *devicesPage();
  QWidget *sessionPage();
  QWidget *wirelessPage();
  QWidget *capturesPage();
  QWidget *inputPage();
  QWidget *settingsPage();
  QWidget *diagnosticsPage();
  void updateDevices();
  void updateActions();
  void updateCaptures();
  void savePreferences();
  void launch(bool record);
  QString selectedSerial() const;
  void captureScreen();
  bool ensureCaptureDirectory();
};
