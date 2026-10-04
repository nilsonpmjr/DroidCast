#include "engine.h"
#include "ui.h"
#include "window.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QImage>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

class HubTests : public QObject {
  Q_OBJECT
  QString fakeTool() const {
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + "/fake-tool.exe";
#else
    return QCoreApplication::applicationDirPath() + "/fake-tool";
#endif
  }
private slots:
  void initTestCase() {
    QCoreApplication::setOrganizationName("DroidCast-tests");
    QCoreApplication::setApplicationName("DroidCast-tests");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QVERIFY(storage.isValid());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       storage.path());
    const auto runtime = storage.filePath("test-runtime");
    QVERIFY(QDir().mkpath(runtime));
    qputenv("DROIDCAST_RUNTIME_DIR", runtime.toUtf8());
    const auto prefs = bundledPreferences();
    QVERIFY(QFile::copy(fakeTool(), prefs.adb));
    QVERIFY(QFile::copy(fakeTool(), prefs.scrcpy));
    QFile server(prefs.server);
    QVERIFY(server.open(QIODevice::WriteOnly));
    server.write("test server");
    server.close();
    applyTheme(*qobject_cast<QApplication *>(QCoreApplication::instance()));
  }
  void init() { qunsetenv("HUB_TEST_MODE"); }
  void inputModesOnlyEmitCompatibleOptions() {
    Preferences prefs;
    prefs.options = {{"key-injection", "prefer-text"},
                     {"no-key-repeat", true},
                     {"no-mouse-hover", true},
                     {"gamepad", "uhid"}};
    auto args = mirrorArguments("PHONE123", prefs);
    QVERIFY(args.contains("--prefer-text"));
    QVERIFY(!args.contains("--raw-key-events"));
    QVERIFY(args.contains("--no-key-repeat"));
    QVERIFY(args.contains("--no-mouse-hover"));
    QVERIFY(args.contains("--gamepad=uhid"));
    prefs.options["key-injection"] = "raw-key-events";
    args = mirrorArguments("PHONE123", prefs);
    QVERIFY(args.contains("--raw-key-events"));
    QVERIFY(!args.contains("--prefer-text"));
    prefs.keyboard = prefs.mouse = "uhid";
    args = mirrorArguments("PHONE123", prefs);
    QVERIFY(!args.contains("--raw-key-events"));
    QVERIFY(!args.contains("--no-key-repeat"));
    QVERIFY(!args.contains("--no-mouse-hover"));
    prefs.options["no-control"] = true;
    QVERIFY(!mirrorArguments("PHONE123", prefs).contains("--gamepad=uhid"));
    prefs.keyboard = prefs.mouse = "disabled";
    QSettings settings(storage.filePath("inputs.ini"), QSettings::IniFormat);
    prefs.save(settings);
    const auto restored = Preferences::load(settings);
    QCOMPARE(restored.keyboard, QString("disabled"));
    QCOMPARE(restored.mouse, QString("disabled"));
  }
  void recordingOptionsOnlyApplyToRecordings() {
    Preferences prefs;
    prefs.options = {{"record-format", "mp4"},
                     {"record-orientation", "90"},
                     {"time-limit", 60}};
    auto args = mirrorArguments("PHONE123", prefs);
    QVERIFY(!args.contains("--record-format=mp4"));
    QVERIFY(!args.contains("--record-orientation=90"));
    QVERIFY(args.contains("--time-limit=60"));
    QCOMPARE(recordingFormat(prefs), QString("mp4"));
    args = mirrorArguments("PHONE123", prefs, "file.mp4");
    QVERIFY(args.contains("--record-format=mp4"));
    QVERIFY(args.contains("--record-orientation=90"));
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.preferences.options = prefs.options;
    engine.preferences.options["audio-codec"] = "raw";
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy messages(&engine, &Engine::message);
    QVERIFY(!engine.start("PHONE123", "file.mp4"));
    QVERIFY(messages.last().first().toString().contains("MP4 cannot"));
    QVERIFY(!engine.running());
    engine.preferences.audio = false;
    QVERIFY(engine.start("PHONE123", "file.mp4"));
    engine.stop();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
  }
  void inputWidgetsRespectModeAndReadOnly() {
    QSettings settings;
    auto prefs = bundledPreferences();
    prefs.save(settings);
    Window window;
    QComboBox *keyboard = nullptr;
    for (auto *combo : window.findChildren<QComboBox *>())
      if (combo->accessibleName() == "Keyboard simulation")
        keyboard = combo;
    QVERIFY(keyboard);
    auto *repeat = window.findChild<QCheckBox *>("option-no-key-repeat");
    QVERIFY(repeat && repeat->isEnabled());
    keyboard->setCurrentIndex(keyboard->findData("uhid"));
    QVERIFY(!repeat->isEnabled());
    keyboard->setCurrentIndex(keyboard->findData("sdk"));
    QVERIFY(repeat->isEnabled());
    auto *readOnly = window.findChild<QCheckBox *>("option-no-control");
    readOnly->setChecked(true);
    QVERIFY(!repeat->isEnabled());
    QVERIFY(!window.findChild<QComboBox *>("option-gamepad")->isEnabled());
    readOnly->setChecked(false);
    QVERIFY(window.findChild<QComboBox *>("option-gamepad")->isEnabled());
  }
  void lifecycleUsesFramesAndFinalizesRecording() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy messages(&engine, &Engine::message);
    QVERIFY(engine.start("PHONE123", storage.filePath("session.mp4")));
    QCOMPARE(engine.sessionState(), Engine::SessionState::Starting);
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    engine.stop();
    QCOMPARE(engine.sessionState(), Engine::SessionState::Stopping);
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QCOMPARE(engine.sessionState(), Engine::SessionState::Ended);
    QVERIFY(messages.last().first().toString().contains("Recording finalized"));
    qputenv("HUB_TEST_MODE", "no-frame");
    QVERIFY(engine.start("PHONE123"));
    QTest::qWait(200);
    QCOMPARE(engine.sessionState(), Engine::SessionState::Starting);
    engine.stop();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
  }
  void lifecyclePreservesFailureAndDisconnect() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy messages(&engine, &Engine::message);
    qputenv("HUB_TEST_MODE", "recording-error");
    QVERIFY(engine.start("PHONE123", storage.filePath("failed.mp4")));
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    engine.stop();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QCOMPARE(engine.sessionState(), Engine::SessionState::Failed);
    QVERIFY(messages.last().first().toString().contains("incomplete"));
    qputenv("HUB_TEST_MODE", "disconnect");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QCOMPARE(engine.sessionState(), Engine::SessionState::Disconnected);
    qputenv("HUB_TEST_MODE", "ignore-quit");
    QVERIFY(engine.start("PHONE123"));
    engine.stop();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 7000);
    QCOMPARE(engine.sessionState(), Engine::SessionState::Failed);
    QVERIFY(messages.last().first().toString().contains("forced"));
  }
  void advancedSettingsAreValidatedAndPersisted() {
    QSettings settings(storage.filePath("advanced.ini"), QSettings::IniFormat);
    Preferences prefs;
    prefs.options = {{"audio-codec", "aac"},
                     {"video-buffer", 300},
                     {"fullscreen", true},
                     {"unknown", "bad"}};
    prefs.save(settings);
    auto restored = Preferences::load(settings);
    auto args = mirrorArguments("PHONE123", restored);
    QVERIFY(args.contains("--audio-codec=aac"));
    QVERIFY(args.contains("--video-buffer=300"));
    QVERIFY(args.contains("--fullscreen"));
    QVERIFY(!args.join(' ').contains("unknown"));
    restored.options["audio-codec"] = "invalid;value";
    restored.options["video-buffer"] = -10;
    args = mirrorArguments("PHONE123", restored);
    QVERIFY(!args.join(' ').contains("invalid"));
    QVERIFY(!args.contains("--video-buffer=-10"));
    restored.options["no-control"] = true;
    restored.options["show-touches"] = true;
    restored.awake = restored.screenOff = true;
    args = mirrorArguments("PHONE123", restored);
    QVERIFY(args.contains("--no-control"));
    QVERIFY(!args.contains("--show-touches"));
    QVERIFY(!args.contains("--stay-awake"));
    QVERIFY(!args.contains("--turn-screen-off"));
    restored.audio = false;
    QVERIFY(
        !mirrorArguments("PHONE123", restored).contains("--audio-codec=aac"));
  }
  void phoneControlsTargetDeviceAndRespectReadOnly() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy logs(&engine, &Engine::log);
    QSignalSpy messages(&engine, &Engine::message);
    engine.phoneAction("LOCKED", Engine::PhoneAction::Home);
    QVERIFY(!engine.deviceBusy());
    engine.phoneAction("PHONE123", Engine::PhoneAction::Home);
    QTRY_VERIFY_WITH_TIMEOUT(!engine.deviceBusy(), 3000);
    QVERIFY(!logs.isEmpty());
    QVERIFY(logs.last().first().toString().contains(
        "-s|PHONE123|shell|input|keyevent|KEYCODE_HOME"));
    engine.preferences.options["no-control"] = true;
    QVERIFY(engine.start("PHONE123"));
    engine.preferences.options["no-control"] = false;
    QVERIFY(!engine.controlAllowed());
    engine.phoneAction("PHONE123", Engine::PhoneAction::Power);
    QVERIFY(!engine.deviceBusy());
    QVERIFY(messages.last().first().toString().contains("read-only"));
    engine.installApk("PHONE123", "missing.apk");
    QVERIFY(messages.last().first().toString().contains("read-only"));
    engine.stop();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 6000);
    QVERIFY(engine.controlAllowed());
  }
  void settingsSearchFiltersGroups() {
    Window window;
    window.showPage(5);
    auto *search = window.findChild<QLineEdit *>("settingsSearch");
    QVERIFY(search);
    search->setText("video-buffer");
    QVERIFY(!window.findChild<QWidget *>("option-video-buffer")
                 ->parentWidget()
                 ->isHidden());
    QVERIFY(window.findChild<QWidget *>("option-audio-codec")
                ->parentWidget()
                ->isHidden());
    search->clear();
    QVERIFY(!window.findChild<QWidget *>("option-audio-codec")
                 ->parentWidget()
                 ->isHidden());
    auto *category = window.findChild<QComboBox *>("settingsCategory");
    QVERIFY(category);
    category->setCurrentText("Audio");
    QVERIFY(window.findChild<QWidget *>("option-video-buffer")
                ->parentWidget()
                ->isHidden());
    QVERIFY(!window.findChild<QWidget *>("option-audio-codec")
                 ->parentWidget()
                 ->isHidden());
    search->setText("fullscreen");
    QVERIFY(window.findChild<QWidget *>("option-fullscreen")
                ->parentWidget()
                ->isHidden());
    category->setCurrentText("Window");
    QVERIFY(!window.findChild<QWidget *>("option-fullscreen")
                 ->parentWidget()
                 ->isHidden());
    int controls = 0;
    for (auto *button : window.findChildren<QPushButton *>())
      if (button->property("phoneControl").toBool())
        ++controls;
    QCOMPARE(controls, 6);
  }
  void parse() {
    auto devices = parseDevices(
        "* daemon started successfully *\nList of devices attached\n"
        "USB1 device usb:1 model:Galaxy_S24 transport_id:1\n"
        "192.168.1.2:5555 offline\nLOCKED unauthorized\n"
        "USB2 no permissions (user in plugdev group); see documentation\n"
        "adb-123._adb-tls-connect._tcp device model:Pixel\n");
    QCOMPARE(devices.size(), 5);
    QCOMPARE(devices[0].model, "Galaxy S24");
    QVERIFY(devices[0].ready());
    QCOMPARE(devices[1].connection, "Wireless");
    QVERIFY(!devices[2].ready());
    QCOMPARE(devices[3].state, "no permissions");
    QCOMPARE(devices[4].connection, "Wireless");
    QVERIFY(parseDevices("List of devices attached\n\n").isEmpty());
  }
  void argumentsAreSeparate() {
    Preferences p;
    p.audio = false;
    p.awake = true;
    p.size = 0;
    auto args = mirrorArguments("phone; $(echo nope)", p,
                                "/folder with spaces/video.mkv");
    QCOMPARE(args.first(), "--serial=phone; $(echo nope)");
    QVERIFY(args.contains("--record=/folder with spaces/video.mkv"));
    QVERIFY(args.contains("--no-audio"));
    QVERIFY(args.contains("--stay-awake"));
    QVERIFY(!args.contains("--max-size=0"));
    QVERIFY(args.contains("--video-bit-rate=8M"));
  }
  void endpoints() {
    for (const auto &s :
         {"192.168.1.1:5555", "phone.local:12345", "[::1]:5555"})
      QVERIFY(validEndpoint(s));
    for (const auto &s : {"", "-x:5555", "host:0", "host:65536",
                          "host:5555; ls", "host", "host:1\n"})
      QVERIFY(!validEndpoint(s));
  }
  void persistence() {
    QSettings settings(storage.filePath("preferences.ini"),
                       QSettings::IniFormat);
    Preferences p;
    p.adb = "/a path/adb";
    p.fps = 120;
    p.screenOff = true;
    p.keyboard = "uhid";
    p.save(settings);
    auto restored = Preferences::load(settings);
    QCOMPARE(restored.adb, bundledPreferences().adb);
    QCOMPARE(restored.fps, 120);
    QVERIFY(!settings.contains("tools/adb"));
    QVERIFY(restored.screenOff);
    QCOMPARE(restored.keyboard, "uhid");
    settings.setValue("session/fps", -5);
    settings.setValue("session/keyboard", "bad");
    restored = Preferences::load(settings);
    QCOMPARE(restored.fps, 1);
    QCOMPARE(restored.keyboard, "sdk");
  }
  void discoverAndRun() {
    Engine engine;
    engine.preferences.adb = fakeTool();
    engine.preferences.scrcpy = fakeTool();
    QSignalSpy found(&engine, &Engine::devicesChanged);
    engine.refresh();
    QVERIFY(found.wait());
    QCOMPARE(engine.devices.size(), 2);
    QVERIFY(!engine.start("LOCKED"));
    QVERIFY(engine.start("PHONE123"));
    QVERIFY(!engine.start("PHONE123"));
    QTRY_VERIFY(engine.running());
    engine.stop();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 7000);
    QTRY_VERIFY(engine.activeSerial.isEmpty());
  }
  void failuresClearState() {
    Engine engine;
    engine.preferences.adb = fakeTool();
    engine.preferences.scrcpy = fakeTool();
    QSignalSpy found(&engine, &Engine::devicesChanged);
    engine.refresh();
    QVERIFY(found.wait());
    qputenv("HUB_TEST_MODE", "mirror-fail");
    QSignalSpy messages(&engine, &Engine::message);
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(!engine.running());
    QTRY_VERIFY(engine.activeSerial.isEmpty());
    QVERIFY(messages.last()[0].toString().contains("error"));
    qputenv("HUB_TEST_MODE", "scan-fail");
    engine.refresh();
    QVERIFY(found.wait());
    QVERIFY(engine.devices.isEmpty());
    engine.preferences.adb = "/nonexistent/hub-test-adb";
    engine.refresh();
    QVERIFY(engine.devices.isEmpty());
  }
  void wirelessResultsAndSecret() {
    Engine engine;
    engine.preferences.adb = fakeTool();
    QSignalSpy messages(&engine, &Engine::message);
    engine.connectWireless("phone.local:5555", "123456");
    QTRY_VERIFY(!engine.wirelessBusy());
    QTRY_VERIFY(messages.last()[0].toString().contains("Successfully paired"));
    for (const auto &args : messages)
      QVERIFY(!args[0].toString().contains("123456"));
    qputenv("HUB_TEST_MODE", "wireless-fail");
    engine.connectWireless("phone.local:5555");
    QTRY_VERIFY(!engine.wirelessBusy());
    QTRY_VERIFY(
        messages.last()[0].toString().contains("Wireless request failed"));
  }
  void scanTimeoutClearsStaleDevices() {
    Engine engine;
    engine.preferences.adb = fakeTool();
    engine.devices = {{"STALE", "device", "Previously connected phone", "USB"}};
    qputenv("HUB_TEST_MODE", "hang");
    QSignalSpy found(&engine, &Engine::devicesChanged);
    QSignalSpy messages(&engine, &Engine::message);
    engine.refresh();
    QVERIFY(found.wait(12000));
    QVERIFY(engine.devices.isEmpty());
    QVERIFY(!engine.scanning());
    QVERIFY(messages.last()[0].toString().contains("timed out"));
  }
  void welcomeDoesNotAutoMirror() {
    QSettings settings;
    Preferences p = bundledPreferences();
    p.mediaDirectory = storage.filePath("captures");
    p.save(settings);
    Window window;
    window.show();
    auto *list = window.findChild<QListWidget *>("devices");
    auto *start = window.findChild<QPushButton *>("startMirror");
    auto *stop = window.findChild<QPushButton *>("stopMirror");
    QVERIFY(list && start && stop);
    QTRY_COMPARE(list->count(), 2);
    QVERIFY(start->isEnabled());
    QVERIFY(!stop->isEnabled());
    list->setCurrentRow(1);
    QVERIFY(!start->isEnabled());
    list->setCurrentRow(0);
    QTest::mouseClick(start, Qt::LeftButton);
    QTRY_VERIFY(stop->isEnabled());
    QVERIFY(!start->isEnabled());
    window.showPage(0);
    QTest::mouseClick(stop, Qt::LeftButton);
    QTRY_VERIFY_WITH_TIMEOUT(!stop->isEnabled(), 7000);
    QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(), 7000);
  }
  void captureWritesOnlyValidImages() {
    Engine engine;
    engine.preferences.adb = fakeTool();
    engine.devices = {{"PHONE123", "device", "Test phone", "USB"}};
    QSignalSpy saved(&engine, &Engine::captureSaved);
    const auto path = storage.filePath("a screenshot.png");
    engine.capture("PHONE123", path);
    QVERIFY(saved.wait());
    QVERIFY(!QImage(path).isNull());
    const auto badPath = storage.filePath("invalid.png");
    qputenv("HUB_TEST_MODE", "invalid-png");
    engine.capture("PHONE123", badPath);
    QTRY_VERIFY(!engine.deviceBusy());
    QVERIFY(!QFileInfo::exists(badPath));
    QCOMPARE(saved.size(), 1);
  }
  void installationDetectsFailureWithZeroExit() {
    Engine engine;
    engine.preferences.adb = fakeTool();
    engine.devices = {{"PHONE123", "device", "Test phone", "USB"}};
    QFile apk(storage.filePath("test.apk"));
    QVERIFY(apk.open(QIODevice::WriteOnly));
    apk.write("test");
    apk.close();
    QSignalSpy messages(&engine, &Engine::message);
    engine.installApk("PHONE123", apk.fileName());
    QTRY_VERIFY(!engine.deviceBusy());
    QTRY_VERIFY(
        messages.last()[0].toString().contains("Application installed"));
    qputenv("HUB_TEST_MODE", "install-fail");
    engine.installApk("PHONE123", apk.fileName());
    QTRY_VERIFY(!engine.deviceBusy());
    QTRY_VERIFY(messages.last()[0].toString().contains("Installation failed"));
  }
  void settingsHaveNoExecutablePickersAndTogglesStayInSync() {
    Window window;
    window.showPage(5);
    for (auto *edit : window.findChildren<QLineEdit *>()) {
      QVERIFY(!edit->accessibleName().contains("executable"));
      QVERIFY(!edit->text().contains("/runtime/"));
    }
    QList<QCheckBox *> audio;
    for (auto *check : window.findChildren<QCheckBox *>())
      if (check->text() == "Forward audio")
        audio.append(check);
    QCOMPARE(audio.size(), 2);
    audio[0]->setChecked(!audio[0]->isChecked());
    QCOMPARE(audio[0]->isChecked(), audio[1]->isChecked());
    QCOMPARE(window.windowTitle(), "DroidCast Desktop");
    QCOMPARE(window.findChild<QListWidget *>("navigation")->count(), 7);
  }

private:
  QTemporaryDir storage;
};
QTEST_MAIN(HubTests)
#include "test_hub.moc"
