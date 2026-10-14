#include "livewindow.h"
#include <QAction>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

class LiveTests : public QObject {
  Q_OBJECT
  QTemporaryDir storage;
  void configure(Engine &engine) {
#ifdef Q_OS_WIN
    const auto tool =
        QCoreApplication::applicationDirPath() + "/compact-fake-tool.exe";
#else
    const auto tool =
        QCoreApplication::applicationDirPath() + "/compact-fake-tool";
#endif
    engine.preferences.adb = engine.preferences.scrcpy = tool;
    engine.preferences.server = storage.filePath("server");
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"},
                      {"LOCKED", "unauthorized", "Locked", "USB"}};
  }
  void snapshot(Engine &engine, const QList<Device> &devices,
                bool success = true) {
    engine.devices = devices;
    emit engine.devicesChanged();
    emit engine.deviceScanCompleted(success);
  }
private slots:
  void initTestCase() {
    QVERIFY(storage.isValid());
    QFile server(storage.filePath("server"));
    QVERIFY(server.open(QIODevice::WriteOnly));
    server.write("fake server");
  }
  void init() { qunsetenv("HUB_TEST_MODE"); }
  void cleanup() {
    qunsetenv("HUB_ENV_LOG");
    qunsetenv("ADB_SERVER_SOCKET");
  }
  void eventStreamHandlesSplitAndCoalescedFrames() {
    Engine engine;
    configure(engine);
    qputenv("HUB_TEST_MODE", "monitor-fragments");
    QSignalSpy snapshots(&engine, &Engine::deviceScanCompleted);
    engine.startDeviceMonitoring();
    QTRY_COMPARE_WITH_TIMEOUT(snapshots.size(), 3, 3000);
    for (const auto &snapshot : snapshots)
      QVERIFY(snapshot.first().toBool());
    QCOMPARE(engine.devices.size(), 1);
    QCOMPARE(engine.devices.first().serial, QString("PHONE123"));
    engine.stopDeviceMonitoring();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.deviceMonitorRunning(), 3000);
  }
  void eventStreamRecovery_data() {
    QTest::addColumn<QString>("mode");
    QTest::newRow("invalid-prefix") << QString("monitor-malformed");
    QTest::newRow("truncated-frame") << QString("monitor-truncated");
    QTest::newRow("failed-process") << QString("monitor-fail");
  }
  void eventStreamRecovery() {
    QFETCH(QString, mode);
    Engine engine;
    configure(engine);
    qputenv("HUB_TEST_MODE", mode.toUtf8());
    QSignalSpy snapshots(&engine, &Engine::deviceScanCompleted);
    QSignalSpy messages(&engine, &Engine::message);
    engine.startDeviceMonitoring();
    QTRY_VERIFY_WITH_TIMEOUT(!snapshots.isEmpty(), 3000);
    QVERIFY(!snapshots.first().first().toBool());
    QVERIFY(engine.devices.isEmpty());
    qunsetenv("HUB_TEST_MODE");
    QTRY_VERIFY_WITH_TIMEOUT(!engine.devices.isEmpty(), 4000);
    QCOMPARE(engine.devices.first().serial, QString("PHONE123"));
    QVERIFY(messages.last().first().toString().contains("recovered"));
    engine.stopDeviceMonitoring();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.deviceMonitorRunning(), 3000);
  }
  void idleMonitorDoesNotLaunchPeriodicQueriesAndStopCancelsRetry() {
    const auto path = storage.filePath("monitor-idle.log");
    qputenv("HUB_ENV_LOG", path.toUtf8());
    Engine engine;
    configure(engine);
    engine.startDeviceMonitoring();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.devices.isEmpty(), 3000);
    QTest::qWait(4500);
    QFile log(path);
    QVERIFY(log.open(QIODevice::ReadOnly));
    const auto lines = log.readAll().split('\n');
    log.close();
    QCOMPARE(lines.size(), 2);
    QVERIFY(lines.first().startsWith("track-devices|-l\t"));
    engine.stopDeviceMonitoring();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.deviceMonitorRunning(), 3000);
    const auto retryPath = storage.filePath("monitor-retry.log");
    qputenv("HUB_ENV_LOG", retryPath.toUtf8());
    qputenv("HUB_TEST_MODE", "monitor-fail");
    QSignalSpy failed(&engine, &Engine::deviceScanCompleted);
    engine.startDeviceMonitoring();
    QTRY_VERIFY_WITH_TIMEOUT(!failed.isEmpty(), 3000);
    engine.stopDeviceMonitoring();
    QTest::qWait(1500);
    QFile retryLog(retryPath);
    QVERIFY(retryLog.open(QIODevice::ReadOnly));
    QCOMPARE(retryLog.readAll().split('\n').size(), 2);
    QVERIFY(!engine.deviceMonitorRunning());
  }
  void eventStreamOffersArrivalAndQuitWaitsForObserver() {
    Engine engine;
    configure(engine);
    qputenv("HUB_TEST_MODE", "monitor-events");
    LiveWindow window(&engine, true);
    QTRY_VERIFY_WITH_TIMEOUT(
        window.findChild<QPushButton *>("liveUsbMirror")->isEnabled(), 3000);
    QVERIFY(!engine.running());
    QCOMPARE(window.selectedSerial(), QString("NEW"));
    QSignalSpy closed(&window, &LiveWindow::shutdownReady);
    window.requestQuit();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.deviceMonitorRunning(), 3000);
    QTRY_COMPARE_WITH_TIMEOUT(closed.size(), 1, 3000);
    QVERIFY(!engine.deviceMonitoring());
  }
  void monitorUsesPinnedRouteAndResumesAfterOtg() {
    Engine engine;
    configure(engine);
    const auto path = storage.filePath("monitor-route.log");
    qputenv("HUB_ENV_LOG", path.toUtf8());
    qputenv("ADB_SERVER_SOCKET", "tcp:inherited:1234");
    engine.startDeviceMonitoring();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.devices.isEmpty(), 3000);
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    engine.preferences.remoteAdb = true;
    engine.preferences.adbServerEndpoint = "127.0.0.1:5038";
    engine.refresh();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.devices.isEmpty(), 3000);
    QFile log(path);
    QVERIFY(log.open(QIODevice::ReadOnly));
    auto data = log.readAll();
    log.close();
    QVERIFY(!data.contains("inherited"));
    QVERIFY(!data.contains("tcp:127.0.0.1:5038"));
    engine.stop();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(!engine.devices.isEmpty(), 3000);
    QVERIFY(log.open(QIODevice::ReadOnly));
    data = log.readAll();
    log.close();
    QVERIFY(data.contains("track-devices|-l\ttcp:127.0.0.1:5038"));
    engine.preferences.remoteAdb = false;
    engine.refresh();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.devices.isEmpty(), 3000);
    QVERIFY(engine.startOtg("USB-OTG-123", true, true, false));
    QTRY_VERIFY_WITH_TIMEOUT(!engine.deviceMonitorRunning(), 3000);
    engine.stop();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(
        engine.deviceMonitorRunning() && !engine.devices.isEmpty(), 3000);
    engine.stopDeviceMonitoring();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.deviceMonitorRunning(), 3000);
  }
  void scanCompletionDistinguishesFailure() {
    Engine engine;
    configure(engine);
    QSignalSpy scanned(&engine, &Engine::deviceScanCompleted);
    engine.refresh();
    QTRY_COMPARE_WITH_TIMEOUT(scanned.size(), 1, 3000);
    QVERIFY(scanned.first().first().toBool());
    qputenv("HUB_TEST_MODE", "scan-fail");
    engine.refresh();
    QTRY_COMPARE_WITH_TIMEOUT(scanned.size(), 2, 3000);
    QVERIFY(!scanned.last().first().toBool());
  }
  void usbBaselineAuthorizationDismissAndReconnect() {
    Engine engine;
    configure(engine);
    LiveWindow window(&engine, false);
    auto *offer = window.findChild<QGroupBox *>("liveUsbOffer");
    auto *mirror = window.findChild<QPushButton *>("liveUsbMirror");
    snapshot(engine, {});
    QVERIFY(offer->isHidden());
    snapshot(engine, {{"NEW", "unauthorized", "New phone", "USB"}});
    QVERIFY(!offer->isHidden());
    QVERIFY(!mirror->isEnabled());
    QVERIFY(!engine.running());
    snapshot(engine, {{"NEW", "device", "New phone", "USB"}});
    QVERIFY(mirror->isEnabled());
    QVERIFY(!engine.running());
    window.findChild<QPushButton *>("liveUsbDismiss")->click();
    snapshot(engine, engine.devices);
    QVERIFY(offer->isHidden());
    snapshot(engine, {});
    snapshot(engine, {{"NEW", "device", "New phone", "USB"}});
    QVERIFY(!offer->isHidden());
  }
  void startupFailureRecoveryWirelessAndRemoteDoNotOffer() {
    Engine engine;
    configure(engine);
    LiveWindow window(&engine, false);
    auto *offer = window.findChild<QGroupBox *>("liveUsbOffer");
    const QList<Device> baseline{{"PHONE123", "device", "Pixel", "USB"}};
    snapshot(engine, baseline);
    QVERIFY(offer->isHidden());
    snapshot(engine, {}, false);
    snapshot(engine, baseline);
    QVERIFY(offer->isHidden());
    snapshot(engine, {baseline.first(),
                      {"10.0.0.1:5555", "device", "Wireless", "Wireless"},
                      {"emulator-5554", "device", "Emulator", "Emulator"}});
    QVERIFY(offer->isHidden());
    engine.preferences.remoteAdb = true;
    snapshot(engine, {{"REMOTE", "device", "Remote phone", "USB"}});
    QVERIFY(offer->isHidden());
    engine.preferences.remoteAdb = false;
    snapshot(engine, baseline);
    QVERIFY(offer->isHidden());
  }
  void usbOfferPinsItsTargetRatherThanCurrentSelection() {
    Engine engine;
    configure(engine);
    LiveWindow window(&engine, false);
    const Device existing{"PHONE123", "device", "Pixel", "USB"};
    snapshot(engine, {existing});
    snapshot(engine, {existing, {"NEW", "device", "Another phone", "USB"}});
    QCOMPARE(window.selectedSerial(), QString("PHONE123"));
    window.findChild<QAction *>("liveUsbMirrorAction")->trigger();
    QCOMPARE(engine.activeSerial, QString("NEW"));
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    window.stopSession();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
  }
  void usbDisconnectAndActiveSessionCannotRedirectOffer() {
    Engine engine;
    configure(engine);
    LiveWindow window(&engine, false);
    const Device existing{"PHONE123", "device", "Pixel", "USB"};
    snapshot(engine, {existing});
    window.startSession();
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    snapshot(engine, {existing, {"NEW", "device", "New phone", "USB"}});
    auto *mirror = window.findChild<QPushButton *>("liveUsbMirror");
    QVERIFY(!mirror->isEnabled());
    mirror->click();
    QCOMPARE(engine.activeSerial, QString("PHONE123"));
    window.stopSession();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QVERIFY(mirror->isEnabled());
    snapshot(engine, {existing});
    QVERIFY(window.findChild<QGroupBox *>("liveUsbOffer")->isHidden());
    mirror->click();
    QVERIFY(!engine.running());
  }
  void usbMultipleArrivalsAndPersistentOptOut() {
    QSettings prefs(storage.filePath("offers.ini"), QSettings::IniFormat);
    prefs.clear();
    Engine engine;
    configure(engine);
    LiveWindow window(&engine, false, {}, &prefs);
    snapshot(engine, {});
    const QList<Device> devices{{"ONE", "device", "One", "USB"},
                                {"TWO", "device", "Two", "USB"}};
    snapshot(engine, devices);
    auto *text = window.findChild<QLabel *>("liveUsbOfferText");
    QVERIFY(text->text().contains("ONE"));
    window.findChild<QPushButton *>("liveUsbDismiss")->click();
    QVERIFY(text->text().contains("TWO"));
    window.setUsbOffersEnabled(false);
    QVERIFY(window.findChild<QGroupBox *>("liveUsbOffer")->isHidden());
    QVERIFY(!prefs.value("ui/usbOffers").toBool());
    snapshot(engine, {});
    snapshot(engine, devices);
    QVERIFY(window.findChild<QGroupBox *>("liveUsbOffer")->isHidden());
    window.setUsbOffersEnabled(true);
    snapshot(engine, devices);
    QVERIFY(window.findChild<QGroupBox *>("liveUsbOffer")->isHidden());
    window.requestQuit();
    snapshot(engine, {});
    snapshot(engine, devices);
    QVERIFY(window.findChild<QGroupBox *>("liveUsbOffer")->isHidden());
  }
  void transferChoiceRejectsDisconnectedAndReattachedTarget() {
    Engine engine;
    configure(engine);
    LiveWindow window(&engine, false);
    snapshot(engine, {});
    const QList<Device> phone{{"PHONE123", "device", "Pixel", "USB"}};
    snapshot(engine, phone);
    QSignalSpy transferMessages(&engine, &Engine::filePushMessage);
    bool acceptedFile = false;
    QTimer::singleShot(0, &window, [&] {
      auto *dialog =
          qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
      if (!dialog)
        return;
      snapshot(engine, {});
      snapshot(engine, phone);
      dialog->selectFile(storage.filePath("server"));
      acceptedFile =
          QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection);
    });
    // Prevent a dialog regression from hanging the test indefinitely.
    QTimer::singleShot(1000, &window, [&] {
      if (auto *dialog =
              qobject_cast<QDialog *>(QApplication::activeModalWidget()))
        dialog->reject();
    });
    window.findChild<QPushButton *>("liveUsbTransfer")->click();
    QVERIFY(acceptedFile);
    QVERIFY(!engine.deviceBusy());
    QVERIFY(transferMessages.isEmpty());
  }
  void unauthorizedCannotStartAndSelectingDoesNotLaunch() {
    Engine engine;
    configure(engine);
    LiveWindow window(&engine, false);
    auto *selection = window.findChild<QComboBox *>("liveDevices");
    selection->setCurrentIndex(1);
    QVERIFY(!window.findChild<QPushButton *>("liveStart")->isEnabled());
    window.startSession();
    QVERIFY(!engine.running());
    selection->setCurrentIndex(0);
    QVERIFY(window.findChild<QAction *>("liveStartAction")->isEnabled());
    QVERIFY(!engine.running());
  }
  void menuLaunchWaitsForFrameAndPinsSerial() {
    Engine engine;
    configure(engine);
    LiveWindow window(&engine, false);
    window.findChild<QAction *>("liveStartAction")->trigger();
    QCOMPARE(engine.activeSerial, QString("PHONE123"));
    QVERIFY(engine.sessionState() != Engine::SessionState::Streaming);
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    QVERIFY(!window.findChild<QComboBox *>("liveDevices")->isEnabled());
    QVERIFY(window.findChild<QPushButton *>("liveFit")->isEnabled());
    QVERIFY(window.findChild<QAction *>("liveFitAction")->isEnabled());
    QSignalSpy results(&engine, &Engine::windowControlMessage);
    window.findChild<QAction *>("liveFitAction")->trigger();
    QTRY_VERIFY_WITH_TIMEOUT(!results.isEmpty(), 3000);
    window.stopSession();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QVERIFY(window.findChild<QComboBox *>("liveDevices")->isEnabled());
  }
  void failureCanRetryAndCloseWaitsForExit() {
    Engine engine;
    configure(engine);
    LiveWindow window(&engine, false);
    qputenv("HUB_TEST_MODE", "mirror-fail");
    window.startSession();
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Failed, 3000);
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    qunsetenv("HUB_TEST_MODE");
    window.startSession();
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    QSignalSpy closed(&window, &LiveWindow::shutdownReady);
    window.close();
    QVERIFY(closed.isEmpty());
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QTRY_COMPARE_WITH_TIMEOUT(closed.size(), 1, 3000);
  }
  void backgroundCloseKeepsSessionAndTrayStopDoesNotQuit() {
    Engine engine;
    configure(engine);
    bool available = true;
    LiveWindow window(&engine, false, [&] { return available; });
    window.setTrayEnabled(true);
    window.setBackgroundEnabled(true);
    window.show();
    window.startSession();
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    QSignalSpy closed(&window, &LiveWindow::shutdownReady);
    window.close();
    QVERIFY(!window.isVisible());
    QVERIFY(engine.running());
    QVERIFY(closed.isEmpty());
    auto *trayMenu = window.findChild<QMenu *>("liveTrayMenu");
    auto *restore = window.findChild<QAction *>("liveRestoreAction");
    QVERIFY(trayMenu->actions().contains(restore));
    restore->trigger();
    QVERIFY(window.isVisible());
    // The tray reuses the same command; stopping a session must not exit the
    // app.
    auto *stop = window.findChild<QAction *>("liveStopAction");
    QVERIFY(trayMenu->actions().contains(stop));
    stop->trigger();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QVERIFY(closed.isEmpty());
    window.requestQuit();
    QCOMPARE(closed.size(), 1);
  }
  void trayLossRestoresAndNoTrayCannotHide() {
    Engine engine;
    configure(engine);
    bool available = true;
    LiveWindow window(&engine, false, [&] { return available; });
    window.setTrayEnabled(true);
    window.setBackgroundEnabled(true);
    window.show();
    window.close();
    QVERIFY(!window.isVisible());
    available = false;
    window.refreshTrayAvailability();
    QVERIFY(window.isVisible());
    QVERIFY(!window.trayUsable());
    QSignalSpy closed(&window, &LiveWindow::shutdownReady);
    window.close();
    QCOMPARE(closed.size(), 1);
  }
  void desktopPreferencesRoundTripAndQuitBypassesBackground() {
    QSettings prefs(storage.filePath("desktop.ini"), QSettings::IniFormat);
    prefs.clear();
    Engine engine;
    configure(engine);
    {
      LiveWindow window(
          &engine, false, [] { return true; }, &prefs);
      window.setTrayEnabled(true);
      window.setBackgroundEnabled(true);
    }
    LiveWindow restored(
        &engine, false, [] { return true; }, &prefs);
    restored.show();
    QVERIFY(restored.trayUsable());
    restored.startSession();
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    QSignalSpy closed(&restored, &LiveWindow::shutdownReady);
    restored.requestQuit();
    QVERIFY(closed.isEmpty());
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QTRY_COMPARE_WITH_TIMEOUT(closed.size(), 1, 3000);
    QVERIFY(prefs.value("ui/background").toBool());
  }
};
QTEST_MAIN(LiveTests)
#include "test_livewindow.moc"
