#include "engine.h"
#include "ui.h"
#include "window.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QStackedWidget>
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
  void virtualDisplayArgumentsAndValidation() {
    Preferences prefs;
    prefs.options = {{"new-display", "1920x1080/240"},
                     {"start-app", "com.android.settings"},
                     {"no-vd-system-decorations", true},
                     {"no-vd-destroy-content", true},
                     {"display-ime-policy", "local"},
                     {"flex-display", true}};
    auto args = mirrorArguments("PHONE123", prefs);
    QVERIFY(!args.join(' ').contains("new-display"));
    QVERIFY(!args.join(' ').contains("start-app"));
    QVERIFY(!args.join(' ').contains("display-ime-policy"));
    QVERIFY(!args.contains("--flex-display"));
    prefs.options["virtual-display"] = true;
    args = mirrorArguments("PHONE123", prefs);
    for (const auto flag :
         {"--new-display=1920x1080/240", "--start-app=com.android.settings",
          "--no-vd-system-decorations", "--no-vd-destroy-content",
          "--display-ime-policy=local", "--flex-display"})
      QVERIFY(args.contains(flag));
    QVERIFY(!args.contains("--virtual-display"));
    QVERIFY(sessionIssue(prefs).message.isEmpty());
    prefs.options["new-display"] = "";
    QVERIFY(mirrorArguments("PHONE123", prefs).contains("--new-display"));
    for (const auto valid : {"", "1920x1080", "1920x1080/240", "/320"})
      QVERIFY(textOptionError("new-display", valid).isEmpty());
    for (const auto invalid :
         {"0x1080", "1920x0", "1920x1080/0", "1920", "/", "65536x1080",
          "1920x1080/240/2", "1920x1080;exit"})
      QVERIFY(!textOptionError("new-display", invalid).isEmpty());
    for (const auto invalid : {"+com.android.settings", "?settings",
                               "com.android.settings --bad", "com..app"})
      QVERIFY(!textOptionError("start-app", invalid).isEmpty());
    prefs.options["display-id"] = 1;
    QCOMPARE(sessionIssue(prefs).key, QString("display-id"));
    prefs.options["display-id"] = 0;
    prefs.options["crop"] = "720:1280:0:0";
    QCOMPARE(sessionIssue(prefs).key, QString("crop"));
    prefs.options["no-control"] = true;
    QVERIFY(sessionIssue(prefs).message.isEmpty());
    args = mirrorArguments("PHONE123", prefs);
    QVERIFY(!args.contains("--flex-display"));
    QVERIFY(!args.join(' ').contains("start-app"));
    QVERIFY(args.contains("--new-display"));
    prefs.options["virtual-display"] = false;
    prefs.options["new-display"] = "invalid but inactive";
    QVERIFY(sessionIssue(prefs).message.isEmpty());
    prefs.options["display-id"] = 1;
    QVERIFY(mirrorArguments("PHONE123", prefs)
                .contains("--display-ime-policy=local"));
    QSettings settings(storage.filePath("virtual.ini"), QSettings::IniFormat);
    prefs.save(settings);
    QCOMPARE(
        Preferences::load(settings).options.value("new-display").toString(),
        QString("invalid but inactive"));
  }
  void alternateDisplayGuardsStayPinnedToTheSession() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.preferences.options = {{"virtual-display", true}, {"display-id", 1}};
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"},
                      {"OTHER", "device", "Other", "USB"}};
    QSignalSpy messages(&engine, &Engine::message);
    QVERIFY(!engine.start("PHONE123"));
    QVERIFY(messages.last().first().toString().contains("existing display ID"));
    engine.preferences.options["display-id"] = 0;
    QVERIFY(engine.start("PHONE123"));
    QVERIFY(engine.usesAlternateDisplay());
    engine.preferences.options["virtual-display"] = false;
    QVERIFY(engine.usesAlternateDisplay());
    engine.phoneAction("PHONE123", Engine::PhoneAction::Home);
    QVERIFY(!engine.deviceBusy());
    QVERIFY(messages.last().first().toString().contains("no command was sent"));
    const auto capture = storage.filePath("wrong-display.png");
    engine.capture("PHONE123", capture);
    QVERIFY(!engine.deviceBusy());
    QVERIFY(!QFileInfo::exists(capture));
    QVERIFY(messages.last().first().toString().contains(
        "primary display was not captured"));
    QVERIFY(engine.captureAllowed("OTHER"));
    engine.stop();
    QTRY_VERIFY(!engine.running());
    QVERIFY(!engine.usesAlternateDisplay());
    QVERIFY(engine.captureAllowed("PHONE123"));
    engine.preferences.options["display-id"] = 2;
    QVERIFY(engine.start("PHONE123"));
    QVERIFY(engine.usesAlternateDisplay());
    engine.stop();
    QTRY_VERIFY(!engine.running());
  }
  void virtualDisplayControlsRespectDependencies() {
    QSettings settings;
    bundledPreferences().save(settings);
    Window window;
    window.resize(920, 680);
    window.show();
    window.showPage(5);
    auto *category = window.findChild<QComboBox *>("settingsCategory");
    QVERIFY(category->findText("Virtual display") >= 0);
    category->setCurrentText("Virtual display");
    auto *enabled = window.findChild<QCheckBox *>("option-virtual-display");
    auto *size = window.findChild<QLineEdit *>("option-new-display");
    auto *app = window.findChild<QLineEdit *>("option-start-app");
    auto *flex = window.findChild<QCheckBox *>("option-flex-display");
    QVERIFY(enabled && size && app && flex);
    QVERIFY(!size->isEnabled());
    enabled->setChecked(true);
    QVERIFY(size->isEnabled());
    QVERIFY(app->isEnabled());
    QVERIFY(flex->isEnabled());
    size->setText("invalid");
    QMetaObject::invokeMethod(size, "editingFinished", Qt::DirectConnection);
    QVERIFY(!window.findChild<QLabel *>("error-new-display")->isHidden());
    enabled->setChecked(false);
    QVERIFY(window.findChild<QLabel *>("error-new-display")->isHidden());
    enabled->setChecked(true);
    QVERIFY(!window.findChild<QLabel *>("error-new-display")->isHidden());
    window.findChild<QCheckBox *>("option-no-control")->setChecked(true);
    QVERIFY(!app->isEnabled());
    QVERIFY(!flex->isEnabled());
    QVERIFY(size->isEnabled());
    auto *scroll = qobject_cast<QScrollArea *>(
        window.findChild<QStackedWidget *>()->currentWidget());
    QVERIFY(scroll);
    QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    bundledPreferences().save(settings);
  }
  void advancedVideoValidationAndArguments() {
    Preferences prefs;
    prefs.options = {{"crop", "1080:1920:0:10"},
                     {"video-encoder", "c2.android.avc.encoder"},
                     {"display-id", 2},
                     {"capture-orientation", "@90"},
                     {"no-downsize-on-error", true}};
    auto args = mirrorArguments("PHONE123", prefs);
    for (const auto &arg :
         {"--crop=1080:1920:0:10", "--video-encoder=c2.android.avc.encoder",
          "--display-id=2", "--capture-orientation=@90",
          "--no-downsize-on-error"})
      QVERIFY(args.contains(arg));
    QSettings settings(storage.filePath("video.ini"), QSettings::IniFormat);
    prefs.save(settings);
    QCOMPARE(Preferences::load(settings).options.value("crop").toString(),
             QString("1080:1920:0:10"));
    for (const auto &invalid : {"0:200:0:0", "100:-5:0:0", "100:100:0",
                                "100:100:0:0;exit", "99999:20:0:0"})
      QVERIFY(!textOptionError("crop", invalid).isEmpty());
    QVERIFY(!textOptionError("video-encoder", "encoder --bad").isEmpty());
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.preferences.options["crop"] = "bad";
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy messages(&engine, &Engine::message);
    QVERIFY(!engine.start("PHONE123"));
    QVERIFY(messages.last().first().toString().contains("Capture crop"));
    QVERIFY(!engine.running());
    QVERIFY(!mirrorArguments("PHONE123", engine.preferences)
                 .join(' ')
                 .contains("--crop=bad"));
  }
  void inspectionIsTargetedAndDoesNotStartMirror() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.preferences.options["no-control"] = true;
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"},
                      {"OTHER", "device", "Other", "USB"}};
    QSignalSpy results(&engine, &Engine::inspectionResult);
    engine.inspectDevice("LOCKED");
    QVERIFY(!engine.inspecting());
    engine.inspectDevice("PHONE123");
    QVERIFY(engine.inspecting());
    QVERIFY(!engine.start("OTHER"));
    QTRY_VERIFY_WITH_TIMEOUT(!engine.inspecting(), 3000);
    QCOMPARE(results.last()[0].toString(), QString("PHONE123"));
    QVERIFY(results.last()[1].toString().contains(
        "--serial=PHONE123|--list-displays|--list-encoders"));
    QVERIFY(results.last()[1].toString().contains("c2.android.avc.encoder"));
    QVERIFY(!engine.running());
    QCOMPARE(engine.sessionState(), Engine::SessionState::Idle);
    qputenv("HUB_TEST_MODE", "inspect-hang");
    engine.inspectDevice("OTHER");
    engine.cancelInspection();
    QTRY_VERIFY_WITH_TIMEOUT(!engine.inspecting(), 3000);
    QCOMPARE(results.last()[0].toString(), QString("OTHER"));
    QVERIFY(results.last()[1].toString().contains("canceled"));
  }
  void inspectionFailuresAreActionable() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy results(&engine, &Engine::inspectionResult);
    for (const auto mode : {"inspect-fail", "inspect-overflow"}) {
      qputenv("HUB_TEST_MODE", mode);
      engine.inspectDevice("PHONE123");
      QTRY_VERIFY_WITH_TIMEOUT(!engine.inspecting(), 3000);
      const auto report = results.last()[1].toString();
      QVERIFY(report.contains("failed") || report.contains("safety limit"));
      QVERIFY(report.size() < 2000);
    }
    engine.preferences.scrcpy = "/nonexistent/inspection-engine";
    engine.inspectDevice("PHONE123");
    QTRY_VERIFY_WITH_TIMEOUT(!engine.inspecting(), 3000);
    QVERIFY(results.last()[1].toString().contains("Could not start"));
  }
  void inspectionTimeoutCanRecover() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy results(&engine, &Engine::inspectionResult);
    qputenv("HUB_TEST_MODE", "inspect-hang");
    engine.inspectDevice("PHONE123");
    QTRY_VERIFY_WITH_TIMEOUT(!engine.inspecting(), 23000);
    QVERIFY(results.last()[1].toString().contains("timed out"));
    qunsetenv("HUB_TEST_MODE");
    engine.inspectDevice("PHONE123");
    QTRY_VERIFY_WITH_TIMEOUT(!engine.inspecting(), 3000);
    QVERIFY(results.last()[1].toString().contains("--display-id=0"));
  }
  void videoTextFieldShowsInlineValidation() {
    QSettings settings;
    bundledPreferences().save(settings);
    Window window;
    window.showPage(5);
    auto *crop = window.findChild<QLineEdit *>("option-crop");
    auto *error = window.findChild<QLabel *>("error-crop");
    QVERIFY(crop && error);
    crop->setText("bad");
    QMetaObject::invokeMethod(crop, "editingFinished", Qt::DirectConnection);
    QVERIFY(!error->isHidden());
    QVERIFY(crop->accessibleDescription().contains("width:height"));
    crop->setText("720:1280:0:0");
    QMetaObject::invokeMethod(crop, "editingFinished", Qt::DirectConnection);
    QVERIFY(error->isHidden());
    auto *inspect = window.findChild<QPushButton *>("inspectDevice");
    QVERIFY(inspect);
    QTRY_VERIFY(inspect->isEnabled());
    inspect->click();
    auto *report = window.findChild<QPlainTextEdit *>("deviceCapabilities");
    QTRY_VERIFY(report->toPlainText().contains("c2.android.avc.encoder"));
    QVERIFY(report->toPlainText().startsWith("Device: PHONE123"));
    bundledPreferences().save(settings);
  }
  void windowToolbarIsLabelledAndRespondsToEngine() {
    QSettings settings;
    auto prefs = bundledPreferences();
    prefs.options["no-control"] = true;
    prefs.save(settings);
    Window window;
    window.resize(920, 680);
    window.show();
    QList<QPushButton *> controls;
    QPushButton *pause = nullptr;
    for (auto *button : window.findChildren<QPushButton *>()) {
      if (!button->property("windowControl").toBool())
        continue;
      controls.append(button);
      QVERIFY(!button->isEnabled());
      QVERIFY(button->accessibleName().startsWith("Mirror window:"));
      if (button->text() == "Pause image")
        pause = button;
    }
    QCOMPARE(controls.size(), 7);
    QVERIFY(pause);
    auto *start = window.findChild<QPushButton *>("startMirror");
    QTRY_VERIFY(start->isEnabled());
    QTest::mouseClick(start, Qt::LeftButton);
    QTRY_VERIFY(pause->isEnabled());
    pause->click();
    QVERIFY(!pause->isEnabled());
    auto *feedback = window.findChild<QLabel *>("windowControlFeedback");
    QTRY_VERIFY(feedback->text().contains("Mirror image paused"));
    QVERIFY(pause->isEnabled());
    for (auto *button : window.findChildren<QPushButton *>())
      if (button->property("phoneControl").toBool())
        QVERIFY(!button->isEnabled());
    window.findChild<QPushButton *>("stopMirror")->click();
    QTRY_VERIFY(start->isEnabled());
    for (auto *button : controls)
      QVERIFY(!button->isEnabled());
    prefs.options.clear();
    prefs.save(settings);
  }
  void windowCommandsRequireReadinessAndWorkReadOnly() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.preferences.options["no-control"] = true;
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy feedback(&engine, &Engine::windowControlMessage);
    QVERIFY(!engine.windowControlsAvailable());
    engine.windowAction(Engine::WindowAction::Pause);
    QVERIFY(feedback.last().first().toString().contains("require"));
    QVERIFY(engine.start("PHONE123"));
    QVERIFY(!engine.windowControlsAvailable());
    QTRY_VERIFY_WITH_TIMEOUT(engine.windowControlsAvailable(), 3000);
    QVERIFY(!engine.controlAllowed());
    for (const auto action :
         {Engine::WindowAction::Fullscreen, Engine::WindowAction::Fit,
          Engine::WindowAction::PixelPerfect, Engine::WindowAction::RotateLeft,
          Engine::WindowAction::RotateRight, Engine::WindowAction::Pause,
          Engine::WindowAction::Resume}) {
      engine.windowAction(action);
      QVERIFY(!engine.windowControlsAvailable());
      QTRY_VERIFY_WITH_TIMEOUT(engine.windowControlsAvailable(), 3000);
      QVERIFY(!feedback.last().first().toString().contains("Waiting"));
    }
    QVERIFY(feedback.last().first().toString().contains("resumed"));
    engine.stop();
    QVERIFY(!engine.windowControlsAvailable());
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
  }
  void windowCommandsHandleUnavailableTimeoutAndOldEngine() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy feedback(&engine, &Engine::windowControlMessage);
    qputenv("HUB_TEST_MODE", "window-unavailable");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.windowControlsAvailable());
    engine.windowAction(Engine::WindowAction::Fit);
    QTRY_VERIFY(engine.windowControlsAvailable());
    QVERIFY(feedback.last().first().toString().contains("unavailable"));
    engine.stop();
    QTRY_VERIFY(!engine.running());
    qputenv("HUB_TEST_MODE", "window-timeout");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.windowControlsAvailable());
    engine.windowAction(Engine::WindowAction::Pause);
    QTRY_VERIFY_WITH_TIMEOUT(
        feedback.last().first().toString().contains("No confirmation"), 4000);
    QVERIFY(!engine.windowControlsAvailable());
    QVERIFY(engine.running());
    engine.stop();
    QTRY_VERIFY(!engine.running());
    qputenv("HUB_TEST_MODE", "old-bridge");
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE(engine.sessionState(), Engine::SessionState::Streaming);
    QVERIFY(!engine.windowControlsAvailable());
    engine.stop();
    QTRY_VERIFY(!engine.running());
  }
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
