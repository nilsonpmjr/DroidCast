#include "engine.h"
#include "ui.h"
#include "window.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSpinBox>
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
  void parsesStructuredCameraCapabilitiesDefensively() {
    const auto cameras = parseCameraCapabilities(
        "noise before report\nList of cameras:\n"
        "    --camera-id=0    (back, 4000x3000, fps={30, 60}, "
        "zoom-range=[1, 8])\n"
        "        - 1920x1080\n        - 1280x720\n"
        "      High speed capture (--camera-high-speed):\n"
        "        - 1280x720 (fps={120, 240})\n"
        "    --camera-id=front-main    (front, 3200x2400, fps={30})\n"
        "        - 1280x720\nList of apps:\nignored\n");
    QCOMPARE(cameras.size(), 2);
    QCOMPARE(cameras[0].id, QString("0"));
    QCOMPARE(cameras[0].facing, QString("back"));
    QCOMPARE(cameras[0].sensorSize, QString("4000x3000"));
    QCOMPARE(cameras[0].zoomRange, QString("[1, 8]"));
    QCOMPARE(cameras[0].frameRates, QStringList({"30", "60"}));
    QCOMPARE(cameras[0].sizes, QStringList({"1920x1080", "1280x720"}));
    QCOMPARE(cameras[0].highSpeedFrameRates.value("1280x720"),
             QStringList({"120", "240"}));
    QCOMPARE(cameras[1].id, QString("front-main"));
    QCOMPARE(cameras[1].frameRates, QStringList({"30"}));
    QVERIFY(parseCameraCapabilities("List of cameras:\n    (none)").isEmpty());
    QVERIFY(parseCameraCapabilities("List of cameras:\n malformed").isEmpty());
    QVERIFY(parseCameraCapabilities("--camera-id=0 (back, 1x1)").isEmpty());
  }
  void parsesDisplayAndEncoderCapabilitiesDefensively() {
    const QString report =
        "prefix\nList of displays:\n"
        "  --display-id=0    (1080x2400)\n"
        "  --display-id=12    (size unknown)\n"
        " malformed\nList of video encoders:\n"
        " --video-codec=h264 --video-encoder=c2.android.avc.encoder"
        "   (hw) [vendor]\n"
        " --video-codec=h265 --video-encoder=OMX.vendor.hevc (hybrid)\n"
        " --video-codec=av1 --video-encoder=c2.android.av1.encoder (sw)\n"
        " invalid\nList of cameras:\n"
        " --display-id=99 (1x1)\n"
        " --video-codec=h264 --video-encoder=ignored\n";
    const auto displays = parseDisplayCapabilities(report);
    QCOMPARE(displays.size(), 2);
    QCOMPARE(displays[0].id, 0);
    QCOMPARE(displays[0].size, QString("1080x2400"));
    QCOMPARE(displays[1].id, 12);
    QCOMPARE(displays[1].size, QString("size unknown"));
    const auto encoders = parseVideoEncoderCapabilities(report);
    QCOMPARE(encoders.size(), 3);
    QCOMPARE(encoders[0].codec, QString("h264"));
    QCOMPARE(encoders[0].name, QString("c2.android.avc.encoder"));
    QCOMPARE(encoders[0].attributes, QString("(hw) [vendor]"));
    QCOMPARE(encoders[1].codec, QString("h265"));
    QCOMPARE(encoders[2].codec, QString("av1"));
    QVERIFY(parseDisplayCapabilities("List of displays:\n (none)").isEmpty());
    QVERIFY(parseVideoEncoderCapabilities("List of video encoders:\n malformed")
                .isEmpty());
    QVERIFY(parseDisplayCapabilities("--display-id=0 (1x1)").isEmpty());
  }
  void parsesInstalledAppsDefensively() {
    const auto apps = parseAppCapabilities(
        "noise\nList of apps:\n"
        " * Settings                      com.android.settings\n"
        " - Calculator                    com.example.calculator\n"
        " - A deliberately long application name\n"
        "                               com.example.longname\n"
        " malformed\nList of cameras:\n"
        " - Ignored                       com.example.ignored\n");
    QCOMPARE(apps.size(), 3);
    QCOMPARE(apps[0].name, QString("Settings"));
    QCOMPARE(apps[0].package, QString("com.android.settings"));
    QVERIFY(apps[0].system);
    QCOMPARE(apps[1].name, QString("Calculator"));
    QVERIFY(!apps[1].system);
    QCOMPARE(apps[2].name, QString("A deliberately long application name"));
    QCOMPARE(apps[2].package, QString("com.example.longname"));
    QVERIFY(parseAppCapabilities("List of apps:\n (none)").isEmpty());
    QVERIFY(parseAppCapabilities("- App  com.example.outside").isEmpty());
  }
  void cameraArgumentsValidationAndDependencies() {
    Preferences prefs;
    prefs.size = 1080;
    prefs.options = {{"video-source", "camera"},  {"camera-id", "0"},
                     {"camera-facing", "front"},  {"camera-size", "1920x1080"},
                     {"camera-ar", "16:9"},       {"camera-fps", 120},
                     {"camera-high-speed", true}, {"camera-torch", true},
                     {"camera-zoom", "2.5"}};
    auto args = mirrorArguments("PHONE123", prefs);
    for (const auto &flag :
         {"--video-source=camera", "--camera-id=0", "--camera-size=1920x1080",
          "--camera-fps=120", "--camera-high-speed", "--camera-torch",
          "--camera-zoom=2.5"})
      QVERIFY(args.contains(flag));
    QVERIFY(!args.contains("--camera-facing=front"));
    QVERIFY(!args.contains("--camera-ar=16:9"));
    QVERIFY(!args.contains("--max-size=1080"));
    QVERIFY(!args.join(' ').contains("virtual-display"));
    QVERIFY(sessionIssue(prefs).message.isEmpty());

    prefs.options["camera-id"] = "";
    prefs.options["camera-size"] = "";
    args = mirrorArguments("PHONE123", prefs);
    QVERIFY(args.contains("--camera-facing=front"));
    QVERIFY(args.contains("--camera-ar=16:9"));
    QVERIFY(args.contains("--max-size=1080"));
    prefs.options["camera-fps"] = 0;
    QCOMPARE(sessionIssue(prefs).key, QString("camera-fps"));
    prefs.options["camera-high-speed"] = false;
    for (const auto pair :
         {qMakePair(QString("camera-id"), QString("front camera")),
          qMakePair(QString("camera-size"), QString("1920 by 1080")),
          qMakePair(QString("camera-ar"), QString("16/9")),
          qMakePair(QString("camera-zoom"), QString("zero"))}) {
      prefs.options[pair.first] = pair.second;
      QCOMPARE(sessionIssue(prefs).key, pair.first);
      prefs.options[pair.first] = "";
    }
    for (const auto value : {"sensor", "16:9", "1.7778", "0.5"})
      QVERIFY2(textOptionError("camera-ar", value).isEmpty(), value);
    prefs.options["video-source"] = "display";
    prefs.options["camera-size"] = "invalid but inactive";
    prefs.options["camera-zoom"] = "also invalid";
    QVERIFY(sessionIssue(prefs).message.isEmpty());
    args = mirrorArguments("PHONE123", prefs);
    QVERIFY(!args.join(' ').contains("camera-"));
    QVERIFY(!args.join(' ').contains("video-source"));
    prefs.options["audio-source"] = "output";
    QVERIFY(
        mirrorArguments("PHONE123", prefs).contains("--audio-source=output"));
    QSettings legacy(storage.filePath("pre-camera.ini"), QSettings::IniFormat);
    legacy.setValue("options/audio-source", "output");
    QCOMPARE(Preferences::load(legacy).options.value("audio-source").toString(),
             QString("auto"));
  }
  void cameraSessionGuardsStayPinnedToTheSession() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.preferences.options["video-source"] = "camera";
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"},
                      {"OTHER", "device", "Other", "USB"}};
    QSignalSpy messages(&engine, &Engine::message);
    QVERIFY(engine.start("PHONE123"));
    QVERIFY(engine.cameraSession());
    QVERIFY(!engine.captureAllowed("PHONE123"));
    QVERIFY(engine.captureAllowed("OTHER"));
    engine.preferences.options["video-source"] = "display";
    QVERIFY(engine.cameraSession());
    engine.phoneAction("PHONE123", Engine::PhoneAction::Home);
    QVERIFY(!engine.deviceBusy());
    QVERIFY(messages.last().first().toString().contains("camera capture"));
    const auto capture = storage.filePath("wrong-camera-source.png");
    engine.capture("PHONE123", capture);
    QVERIFY(!engine.deviceBusy());
    QVERIFY(!QFileInfo::exists(capture));
    QVERIFY(messages.last().first().toString().contains("camera stream"));
    engine.stop();
    QTRY_VERIFY(!engine.running());
    QVERIFY(!engine.cameraSession());
    QVERIFY(engine.captureAllowed("PHONE123"));
  }
  void liveCameraCommandsRequireCapabilityAndControl() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.preferences.options["video-source"] = "camera";
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy feedback(&engine, &Engine::cameraControlMessage);
    QVERIFY(!engine.cameraControlsAvailable());
    engine.cameraAction(Engine::CameraAction::TorchOn);
    QVERIFY(feedback.last().first().toString().contains("active camera"));
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY_WITH_TIMEOUT(engine.cameraControlsAvailable(), 3000);
    for (const auto action :
         {Engine::CameraAction::TorchOn, Engine::CameraAction::TorchOff,
          Engine::CameraAction::ZoomOut, Engine::CameraAction::ZoomIn}) {
      engine.cameraAction(action);
      QVERIFY(!engine.cameraControlsAvailable());
      QTRY_VERIFY_WITH_TIMEOUT(engine.cameraControlsAvailable(), 3000);
      QVERIFY(feedback.last().first().toString().contains("request sent"));
    }
    engine.stop();
    QTRY_VERIFY(!engine.running());

    engine.preferences.options["no-control"] = true;
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    QVERIFY(!engine.cameraControlsAvailable());
    engine.cameraAction(Engine::CameraAction::ZoomIn);
    QVERIFY(feedback.last().first().toString().contains("read-only"));
    engine.stop();
    QTRY_VERIFY(!engine.running());
    engine.preferences.options["no-control"] = false;

    qputenv("HUB_TEST_MODE", "camera-unavailable");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.cameraControlsAvailable());
    engine.cameraAction(Engine::CameraAction::TorchOn);
    QTRY_VERIFY(engine.cameraControlsAvailable());
    QVERIFY(feedback.last().first().toString().contains("unavailable"));
    engine.stop();
    QTRY_VERIFY(!engine.running());

    qputenv("HUB_TEST_MODE", "camera-timeout");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.cameraControlsAvailable());
    engine.cameraAction(Engine::CameraAction::ZoomOut);
    QTRY_VERIFY_WITH_TIMEOUT(
        feedback.last().first().toString().contains("No confirmation"), 4000);
    QVERIFY(!engine.cameraControlsAvailable());
    QVERIFY(engine.windowControlsAvailable());
    engine.stop();
    QTRY_VERIFY(!engine.running());

    qputenv("HUB_TEST_MODE", "old-camera-bridge");
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE(engine.sessionState(), Engine::SessionState::Streaming);
    QVERIFY(engine.windowControlsAvailable());
    QVERIFY(!engine.cameraControlsAvailable());
    engine.stop();
    QTRY_VERIFY(!engine.running());
  }
  void androidCommandsRequireCapabilityAndControl() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy feedback(&engine, &Engine::androidControlMessage);
    QVERIFY(!engine.androidControlsAvailable());
    engine.androidAction(Engine::AndroidAction::DisplayOff);
    QVERIFY(feedback.last().first().toString().contains("waiting"));
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY_WITH_TIMEOUT(engine.androidControlsAvailable(), 3000);
    for (const auto action :
         {Engine::AndroidAction::DisplayOff, Engine::AndroidAction::DisplayOn,
          Engine::AndroidAction::Notifications,
          Engine::AndroidAction::QuickSettings,
          Engine::AndroidAction::CollapsePanels,
          Engine::AndroidAction::RotateDevice,
          Engine::AndroidAction::ResetVideo}) {
      engine.androidAction(action);
      QVERIFY(!engine.androidControlsAvailable());
      QTRY_VERIFY_WITH_TIMEOUT(engine.androidControlsAvailable(), 3000);
      QVERIFY(feedback.last().first().toString().contains("queued"));
    }
    QVERIFY(feedback.last().first().toString().contains("Video reset"));
    engine.stop();
    QTRY_VERIFY(!engine.running());

    engine.preferences.options["no-control"] = true;
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    QVERIFY(!engine.androidControlsAvailable());
    engine.androidAction(Engine::AndroidAction::DisplayOn);
    QVERIFY(feedback.last().first().toString().contains("read-only"));
    engine.stop();
    QTRY_VERIFY(!engine.running());

    engine.preferences.options["no-control"] = false;
    engine.preferences.options["video-source"] = "camera";
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE_WITH_TIMEOUT(engine.sessionState(),
                              Engine::SessionState::Streaming, 3000);
    QVERIFY(!engine.androidControlsAvailable());
    engine.androidAction(Engine::AndroidAction::RotateDevice);
    QVERIFY(feedback.last().first().toString().contains("camera capture"));
    engine.stop();
    QTRY_VERIFY(!engine.running());
  }
  void androidCommandsHandleUnavailableTimeoutAndOldEngine() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy feedback(&engine, &Engine::androidControlMessage);

    qputenv("HUB_TEST_MODE", "android-unavailable");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.androidControlsAvailable());
    engine.androidAction(Engine::AndroidAction::Notifications);
    QTRY_VERIFY(engine.androidControlsAvailable());
    QVERIFY(feedback.last().first().toString().contains("unavailable"));
    engine.stop();
    QTRY_VERIFY(!engine.running());

    qputenv("HUB_TEST_MODE", "android-timeout");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.androidControlsAvailable());
    engine.androidAction(Engine::AndroidAction::RotateDevice);
    QTRY_VERIFY_WITH_TIMEOUT(
        feedback.last().first().toString().contains("No confirmation"), 4000);
    QVERIFY(!engine.androidControlsAvailable());
    QVERIFY(engine.windowControlsAvailable());
    engine.stop();
    QTRY_VERIFY(!engine.running());

    qputenv("HUB_TEST_MODE", "old-android-bridge");
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE(engine.sessionState(), Engine::SessionState::Streaming);
    QVERIFY(engine.windowControlsAvailable());
    QVERIFY(!engine.androidControlsAvailable());
    engine.stop();
    QTRY_VERIFY(!engine.running());
  }
  void clipboardCommandsRequireCapabilityAndControl() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy feedback(&engine, &Engine::clipboardControlMessage);
    QVERIFY(!engine.clipboardControlsAvailable());
    engine.clipboardAction(Engine::ClipboardAction::CopyFromAndroid);
    QVERIFY(feedback.last().first().toString().contains("waiting"));
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY_WITH_TIMEOUT(engine.clipboardControlsAvailable(), 3000);
    engine.clipboardAction(Engine::ClipboardAction::CopyFromAndroid);
    QVERIFY(!engine.clipboardControlsAvailable());
    QTRY_VERIFY(engine.clipboardControlsAvailable());
    QVERIFY(feedback.last().first().toString().contains("copied"));
    engine.clipboardAction(Engine::ClipboardAction::PasteToAndroid);
    QTRY_VERIFY(engine.clipboardControlsAvailable());
    QVERIFY(feedback.last().first().toString().contains("paste request"));
    engine.stop();
    QTRY_VERIFY(!engine.running());

    engine.preferences.options["no-control"] = true;
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE(engine.sessionState(), Engine::SessionState::Streaming);
    QVERIFY(!engine.clipboardControlsAvailable());
    engine.clipboardAction(Engine::ClipboardAction::PasteToAndroid);
    QVERIFY(feedback.last().first().toString().contains("read-only"));
    engine.stop();
    QTRY_VERIFY(!engine.running());

    engine.preferences.options["no-control"] = false;
    engine.preferences.options["video-source"] = "camera";
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE(engine.sessionState(), Engine::SessionState::Streaming);
    QVERIFY(!engine.clipboardControlsAvailable());
    engine.clipboardAction(Engine::ClipboardAction::CopyFromAndroid);
    QVERIFY(feedback.last().first().toString().contains("camera capture"));
    engine.stop();
    QTRY_VERIFY(!engine.running());
  }
  void clipboardCommandsHandleUnavailableTimeoutAndOldEngine() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy feedback(&engine, &Engine::clipboardControlMessage);

    qputenv("HUB_TEST_MODE", "clipboard-unavailable");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.clipboardControlsAvailable());
    engine.clipboardAction(Engine::ClipboardAction::PasteToAndroid);
    QTRY_VERIFY(engine.clipboardControlsAvailable());
    QVERIFY(feedback.last().first().toString().contains("unavailable"));
    engine.stop();
    QTRY_VERIFY(!engine.running());

    qputenv("HUB_TEST_MODE", "clipboard-timeout");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.clipboardControlsAvailable());
    engine.clipboardAction(Engine::ClipboardAction::CopyFromAndroid);
    QTRY_VERIFY_WITH_TIMEOUT(
        feedback.last().first().toString().contains("No clipboard"), 4000);
    QVERIFY(!engine.clipboardControlsAvailable());
    QVERIFY(engine.windowControlsAvailable());
    engine.stop();
    QTRY_VERIFY(!engine.running());

    qputenv("HUB_TEST_MODE", "old-clipboard-bridge");
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE(engine.sessionState(), Engine::SessionState::Streaming);
    QVERIFY(engine.androidControlsAvailable());
    QVERIFY(!engine.clipboardControlsAvailable());
    engine.stop();
    QTRY_VERIFY(!engine.running());
  }
  void fpsMeasurementReportsSamplesAndWorksReadOnly() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.preferences.options["no-control"] = true;
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy feedback(&engine, &Engine::fpsControlMessage);
    QVERIFY(!engine.fpsControlsAvailable());
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.fpsControlsAvailable());
    engine.fpsAction(Engine::FpsAction::Start);
    QTRY_VERIFY(feedback.last().first().toString().contains("58 rendered fps"));
    QVERIFY(engine.fpsMeasuring());
    QVERIFY(engine.fpsControlsAvailable());
    engine.fpsAction(Engine::FpsAction::Stop);
    QTRY_VERIFY(feedback.last().first().toString().contains("stopped"));
    QVERIFY(!engine.fpsMeasuring());
    engine.stop();
    QTRY_VERIFY(!engine.running());

    engine.preferences.options["no-control"] = false;
    engine.preferences.options["video-source"] = "camera";
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.fpsControlsAvailable());
    engine.fpsAction(Engine::FpsAction::Start);
    QTRY_VERIFY(feedback.last().first().toString().contains("2 skipped"));
    engine.stop();
    QTRY_VERIFY(!engine.running());
    QVERIFY(!engine.fpsMeasuring());

    engine.preferences.options["video-source"] = "display";
    qputenv("HUB_TEST_MODE", "fps-zero");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.fpsControlsAvailable());
    engine.fpsAction(Engine::FpsAction::Start);
    QTRY_VERIFY(feedback.last().first().toString().contains("0 rendered fps"));
    QVERIFY(!feedback.last().first().toString().contains("skipped"));
    engine.fpsAction(Engine::FpsAction::Stop);
    engine.stop();
    QTRY_VERIFY(!engine.running());

    qputenv("HUB_TEST_MODE", "fps-disconnect");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.fpsControlsAvailable());
    engine.fpsAction(Engine::FpsAction::Start);
    QTRY_VERIFY(engine.fpsMeasuring());
    QTRY_VERIFY_WITH_TIMEOUT(!engine.running(), 3000);
    QCOMPARE(engine.sessionState(), Engine::SessionState::Disconnected);
    QVERIFY(!engine.fpsMeasuring());
  }
  void fpsMeasurementHandlesUnavailableTimeoutAndOldEngine() {
    Engine engine;
    engine.preferences = bundledPreferences();
    engine.devices = {{"PHONE123", "device", "Pixel", "USB"}};
    QSignalSpy feedback(&engine, &Engine::fpsControlMessage);

    qputenv("HUB_TEST_MODE", "fps-unavailable");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.fpsControlsAvailable());
    engine.fpsAction(Engine::FpsAction::Start);
    QTRY_VERIFY(engine.fpsControlsAvailable());
    QVERIFY(feedback.last().first().toString().contains("unavailable"));
    engine.stop();
    QTRY_VERIFY(!engine.running());

    qputenv("HUB_TEST_MODE", "fps-timeout");
    QVERIFY(engine.start("PHONE123"));
    QTRY_VERIFY(engine.fpsControlsAvailable());
    engine.fpsAction(Engine::FpsAction::Start);
    QTRY_VERIFY_WITH_TIMEOUT(
        feedback.last().first().toString().contains("No FPS"), 4000);
    QVERIFY(!engine.fpsControlsAvailable());
    QVERIFY(engine.windowControlsAvailable());
    engine.stop();
    QTRY_VERIFY(!engine.running());

    qputenv("HUB_TEST_MODE", "old-fps-bridge");
    QVERIFY(engine.start("PHONE123"));
    QTRY_COMPARE(engine.sessionState(), Engine::SessionState::Streaming);
    QVERIFY(engine.windowControlsAvailable());
    QVERIFY(!engine.fpsControlsAvailable());
    engine.stop();
    QTRY_VERIFY(!engine.running());
  }
  void fpsControlsAreLabelledAndStateful() {
    QSettings settings;
    bundledPreferences().save(settings);
    Window window;
    window.resize(920, 680);
    window.show();
    window.showPage(1);
    auto *feedback = window.findChild<QLabel *>("fpsControlFeedback");
    QList<QPushButton *> controls;
    QPushButton *startMeasurement = nullptr, *stopMeasurement = nullptr;
    for (auto *button : window.findChildren<QPushButton *>()) {
      if (!button->property("fpsControl").toBool())
        continue;
      controls.append(button);
      QVERIFY(button->accessibleName().startsWith("Rendered FPS:"));
      if (button->property("fpsStart").toBool())
        startMeasurement = button;
      else
        stopMeasurement = button;
    }
    QCOMPARE(controls.size(), 2);
    QVERIFY(feedback && startMeasurement && stopMeasurement);
    QVERIFY(!startMeasurement->isEnabled());
    auto *start = window.findChild<QPushButton *>("startMirror");
    QTRY_VERIFY(start->isEnabled());
    start->click();
    QTRY_VERIFY(startMeasurement->isEnabled());
    QVERIFY(!stopMeasurement->isEnabled());
    startMeasurement->click();
    QTRY_VERIFY(feedback->text().contains("58 rendered fps"));
    QVERIFY(!startMeasurement->isEnabled());
    QVERIFY(stopMeasurement->isEnabled());
    stopMeasurement->click();
    QTRY_VERIFY(feedback->text().contains("stopped"));
    QVERIFY(startMeasurement->isEnabled());
    auto *scroll = qobject_cast<QScrollArea *>(
        window.findChild<QStackedWidget *>()->currentWidget());
    QVERIFY(scroll);
    QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    window.findChild<QPushButton *>("stopMirror")->click();
    QTRY_VERIFY(start->isEnabled());
    bundledPreferences().save(settings);
  }
  void cameraSessionPanelIsContextualAndActionable() {
    QSettings settings;
    auto prefs = bundledPreferences();
    prefs.options["video-source"] = "camera";
    prefs.save(settings);
    Window window;
    window.resize(920, 680);
    window.show();
    window.showPage(1);
    auto *panel = window.findChild<QWidget *>("cameraControlsPanel");
    auto *feedback = window.findChild<QLabel *>("cameraControlFeedback");
    QVERIFY(panel && feedback);
    auto *androidPanel = window.findChild<QWidget *>("androidControlsPanel");
    QVERIFY(androidPanel && androidPanel->isHidden());
    QList<QPushButton *> controls;
    QPushButton *torchOn = nullptr;
    for (auto *button : window.findChildren<QPushButton *>()) {
      if (!button->property("cameraControl").toBool())
        continue;
      controls.append(button);
      QVERIFY(!button->isEnabled());
      QVERIFY(button->accessibleName().startsWith("Camera:"));
      if (button->text() == "Torch on")
        torchOn = button;
    }
    QCOMPARE(controls.size(), 4);
    QVERIFY(torchOn);
    QVERIFY(panel->isHidden());
    auto *start = window.findChild<QPushButton *>("startMirror");
    QTRY_VERIFY(start->isEnabled());
    start->click();
    QTRY_VERIFY(!panel->isHidden());
    QVERIFY(androidPanel->isHidden());
    QTRY_VERIFY(torchOn->isEnabled());
    auto *scroll = qobject_cast<QScrollArea *>(
        window.findChild<QStackedWidget *>()->currentWidget());
    QVERIFY(scroll);
    QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    torchOn->click();
    QTRY_VERIFY(feedback->text().contains("request sent"));
    QTRY_VERIFY(torchOn->isEnabled());
    for (auto *button : window.findChildren<QPushButton *>())
      if (button->property("phoneControl").toBool())
        QVERIFY(!button->isEnabled());
    window.findChild<QPushButton *>("stopMirror")->click();
    QTRY_VERIFY(start->isEnabled());
    QVERIFY(panel->isHidden());
    prefs.options.clear();
    prefs.save(settings);
  }
  void androidSessionPanelIsLabelledAndActionable() {
    QSettings settings;
    bundledPreferences().save(settings);
    Window window;
    window.resize(920, 680);
    window.show();
    window.showPage(1);
    auto *panel = window.findChild<QWidget *>("androidControlsPanel");
    auto *feedback = window.findChild<QLabel *>("androidControlFeedback");
    auto *clipboardFeedback =
        window.findChild<QLabel *>("clipboardControlFeedback");
    QVERIFY(panel && feedback && clipboardFeedback);
    QList<QPushButton *> controls;
    QPushButton *rotate = nullptr;
    for (auto *button : window.findChildren<QPushButton *>()) {
      if (!button->property("androidControl").toBool())
        continue;
      controls.append(button);
      QVERIFY(!button->isEnabled());
      QVERIFY(button->accessibleName().startsWith("Android device:"));
      if (button->text() == "Rotate Android")
        rotate = button;
    }
    QCOMPARE(controls.size(), 7);
    QList<QPushButton *> clipboardControls;
    QPushButton *copy = nullptr;
    for (auto *button : window.findChildren<QPushButton *>()) {
      if (!button->property("clipboardControl").toBool())
        continue;
      clipboardControls.append(button);
      QVERIFY(!button->isEnabled());
      QVERIFY(button->accessibleName().startsWith("Clipboard:"));
      if (button->text() == "Copy from Android")
        copy = button;
    }
    QCOMPARE(clipboardControls.size(), 2);
    QVERIFY(copy);
    QVERIFY(rotate);
    QVERIFY(panel->isHidden());
    auto *start = window.findChild<QPushButton *>("startMirror");
    QTRY_VERIFY(start->isEnabled());
    start->click();
    QTRY_VERIFY(!panel->isHidden());
    QTRY_VERIFY(rotate->isEnabled());
    QTRY_VERIFY(copy->isEnabled());
    copy->click();
    QTRY_VERIFY(clipboardFeedback->text().contains("copied"));
    rotate->click();
    QTRY_VERIFY(feedback->text().contains("rotation request queued"));
    auto *scroll = qobject_cast<QScrollArea *>(
        window.findChild<QStackedWidget *>()->currentWidget());
    QVERIFY(scroll);
    QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    window.findChild<QPushButton *>("stopMirror")->click();
    QTRY_VERIFY(start->isEnabled());
    QVERIFY(panel->isHidden());
    bundledPreferences().save(settings);
  }
  void cameraControlsFollowTheSelectedSource() {
    QSettings settings;
    bundledPreferences().save(settings);
    Window window;
    window.resize(920, 680);
    window.show();
    window.showPage(5);
    auto *category = window.findChild<QComboBox *>("settingsCategory");
    QVERIFY(category && category->findText("Camera") >= 0);
    category->setCurrentText("Camera");
    auto *source = window.findChild<QComboBox *>("option-video-source");
    auto *id = window.findChild<QLineEdit *>("option-camera-id");
    auto *facing = window.findChild<QComboBox *>("option-camera-facing");
    auto *size = window.findChild<QLineEdit *>("option-camera-size");
    auto *aspect = window.findChild<QLineEdit *>("option-camera-ar");
    auto *maxSize = window.findChild<QComboBox *>("base-max-size");
    auto *virtualDisplay =
        window.findChild<QCheckBox *>("option-virtual-display");
    auto *keyboard = window.findChild<QCheckBox *>("option-no-key-repeat");
    QVERIFY(source && id && facing && size && aspect && maxSize &&
            virtualDisplay && keyboard);
    QCOMPARE(source->currentText(), QString("display"));
    QVERIFY(!id->isEnabled());
    source->setCurrentText("camera");
    QVERIFY(id->isEnabled());
    QVERIFY(facing->isEnabled());
    QVERIFY(!virtualDisplay->isEnabled());
    QVERIFY(!keyboard->isEnabled());
    id->setText("0");
    QVERIFY(!facing->isEnabled());
    size->setText("1920x1080");
    QVERIFY(!aspect->isEnabled());
    QVERIFY(!maxSize->isEnabled());
    size->setText("bad");
    QMetaObject::invokeMethod(size, "editingFinished", Qt::DirectConnection);
    QVERIFY(!window.findChild<QLabel *>("error-camera-size")->isHidden());
    source->setCurrentText("display");
    QVERIFY(virtualDisplay->isEnabled());
    QVERIFY(maxSize->isEnabled());
    QVERIFY(window.findChild<QLabel *>("error-camera-size")->isHidden());
    bundledPreferences().save(settings);
  }
  void cameraInspectionPopulatesGuidedSelectors() {
    QSettings settings;
    bundledPreferences().save(settings);
    Window window;
    window.resize(920, 680);
    window.show();
    window.showPage(5);
    auto *category = window.findChild<QComboBox *>("settingsCategory");
    auto *source = window.findChild<QComboBox *>("option-video-source");
    auto *camera = window.findChild<QComboBox *>("detectedCamera");
    auto *size = window.findChild<QComboBox *>("detectedCameraSize");
    auto *fps = window.findChild<QComboBox *>("detectedCameraFps");
    auto *idField = window.findChild<QLineEdit *>("option-camera-id");
    auto *sizeField = window.findChild<QLineEdit *>("option-camera-size");
    auto *highSpeed = window.findChild<QCheckBox *>("option-camera-high-speed");
    QVERIFY(category && source && camera && size && fps && idField &&
            sizeField && highSpeed);
    category->setCurrentText("Camera");
    source->setCurrentText("camera");
    QVERIFY(!camera->isEnabled());
    auto *inspect = window.findChild<QPushButton *>("inspectCamera");
    QTRY_VERIFY(inspect->isEnabled());
    inspect->click();
    QTRY_COMPARE(camera->count(), 2);
    QVERIFY(camera->isEnabled());
    QCOMPARE(camera->itemData(1).toString(), QString("0"));
    camera->setCurrentIndex(1);
    QMetaObject::invokeMethod(camera, "activated", Qt::DirectConnection,
                              Q_ARG(int, 1));
    QCOMPARE(idField->text(), QString("0"));
    QVERIFY(size->findData("1920x1080") > 0);
    const int sizeIndex = size->findData("1920x1080");
    size->setCurrentIndex(sizeIndex);
    QMetaObject::invokeMethod(size, "activated", Qt::DirectConnection,
                              Q_ARG(int, sizeIndex));
    QCOMPARE(sizeField->text(), QString("1920x1080"));
    QVERIFY(fps->findData(60) > 0);
    highSpeed->setChecked(true);
    QCOMPARE(size->count(), 2);
    QCOMPARE(size->itemData(1).toString(), QString("1280x720"));
    QCOMPARE(fps->count(), 1);
    size->setCurrentIndex(1);
    QMetaObject::invokeMethod(size, "activated", Qt::DirectConnection,
                              Q_ARG(int, 1));
    QCOMPARE(sizeField->text(), QString("1280x720"));
    QVERIFY(fps->findData(120) > 0);
    QVERIFY(fps->findData(240) > 0);
    QCOMPARE(fps->findData(60), -1);
    auto *scroll = qobject_cast<QScrollArea *>(
        window.findChild<QStackedWidget *>()->currentWidget());
    QVERIFY(scroll);
    QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    idField->setText("manual-vendor-id");
    QMetaObject::invokeMethod(idField, "editingFinished", Qt::DirectConnection);
    QCOMPARE(idField->text(), QString("manual-vendor-id"));
    QCOMPARE(camera->currentIndex(), 0);
    window.findChild<QListWidget *>("devices")->setCurrentRow(1);
    QCOMPARE(camera->count(), 1);
    QVERIFY(!camera->isEnabled());
    bundledPreferences().save(settings);
  }
  void inspectionPopulatesDisplayAndEncoderSelectors() {
    QSettings settings;
    bundledPreferences().save(settings);
    Window window;
    window.resize(920, 680);
    window.show();
    window.showPage(5);
    auto *category = window.findChild<QComboBox *>("settingsCategory");
    auto *display = window.findChild<QComboBox *>("detectedDisplay");
    auto *encoder = window.findChild<QComboBox *>("detectedVideoEncoder");
    auto *displayField = window.findChild<QSpinBox *>("option-display-id");
    auto *encoderField = window.findChild<QLineEdit *>("option-video-encoder");
    auto *codec = window.findChild<QComboBox *>("base-video-codec");
    QVERIFY(category && display && encoder && displayField && encoderField &&
            codec);
    category->setCurrentText("Video");
    QVERIFY(!display->isEnabled());
    auto *inspect = window.findChild<QPushButton *>("inspectDevice");
    QTRY_VERIFY(inspect->isEnabled());
    inspect->click();
    QTRY_COMPARE(display->count(), 3);
    QCOMPARE(encoder->count(), 3);
    QVERIFY(display->isEnabled());
    const int displayIndex = display->findData(2);
    display->setCurrentIndex(displayIndex);
    QMetaObject::invokeMethod(display, "activated", Qt::DirectConnection,
                              Q_ARG(int, displayIndex));
    QCOMPARE(displayField->value(), 2);
    codec->setCurrentText("h265");
    QCOMPARE(encoder->count(), 2);
    QCOMPARE(encoder->itemData(1).toString(),
             QString("c2.android.hevc.encoder"));
    encoderField->setText("vendor.manual.encoder");
    QMetaObject::invokeMethod(encoderField, "editingFinished",
                              Qt::DirectConnection);
    QCOMPARE(encoder->currentIndex(), 0);
    auto *scroll = qobject_cast<QScrollArea *>(
        window.findChild<QStackedWidget *>()->currentWidget());
    QVERIFY(scroll);
    QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    window.findChild<QListWidget *>("devices")->setCurrentRow(1);
    QCOMPARE(display->count(), 1);
    QCOMPARE(encoder->count(), 1);
    QVERIFY(!display->isEnabled());
    bundledPreferences().save(settings);
  }
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
    auto *picker = window.findChild<QComboBox *>("detectedApp");
    auto *flex = window.findChild<QCheckBox *>("option-flex-display");
    QVERIFY(enabled && size && app && picker && flex);
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
    QVERIFY(!picker->isEnabled());
    QVERIFY(!flex->isEnabled());
    QVERIFY(size->isEnabled());
    auto *scroll = qobject_cast<QScrollArea *>(
        window.findChild<QStackedWidget *>()->currentWidget());
    QVERIFY(scroll);
    QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(), 0);
    bundledPreferences().save(settings);
  }
  void installedAppPickerIsSearchableAndTargetScoped() {
    QSettings settings;
    bundledPreferences().save(settings);
    Window window;
    window.resize(920, 680);
    window.show();
    window.showPage(5);
    window.findChild<QComboBox *>("settingsCategory")
        ->setCurrentText("Virtual display");
    auto *picker = window.findChild<QComboBox *>("detectedApp");
    auto *app = window.findChild<QLineEdit *>("option-start-app");
    auto *enabled = window.findChild<QCheckBox *>("option-virtual-display");
    auto *inspect = window.findChild<QPushButton *>("inspectApps");
    QVERIFY(picker && app && enabled && inspect);
    QVERIFY(picker->isEditable());
    QCOMPARE(picker->completer()->filterMode(), Qt::MatchContains);
    QVERIFY(!picker->isEnabled());
    QTRY_VERIFY(inspect->isEnabled());
    inspect->click();
    QTRY_COMPARE(picker->count(), 4);
    enabled->setChecked(true);
    QVERIFY(picker->isEnabled());
    const int index = picker->findData("com.example.calculator");
    QVERIFY(index > 0);
    picker->setCurrentIndex(index);
    QMetaObject::invokeMethod(picker, "activated", Qt::DirectConnection,
                              Q_ARG(int, index));
    QCOMPARE(app->text(), QString("com.example.calculator"));
    app->setText("vendor.manual.app");
    QMetaObject::invokeMethod(app, "editingFinished", Qt::DirectConnection);
    QCOMPARE(picker->currentIndex(), 0);
    window.findChild<QListWidget *>("devices")->setCurrentRow(1);
    QCOMPARE(picker->count(), 1);
    QVERIFY(!picker->isEnabled());
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
        "--serial=PHONE123|--list-displays|--list-encoders|"
        "--list-camera-sizes|--list-apps"));
    QVERIFY(results.last()[1].toString().contains("c2.android.avc.encoder"));
    QVERIFY(results.last()[1].toString().contains("--camera-id=0"));
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
    auto *cameraReport =
        window.findChild<QPlainTextEdit *>("cameraCapabilities");
    QVERIFY(cameraReport);
    QTRY_VERIFY(cameraReport->toPlainText().contains("--camera-id=0"));
    QVERIFY(window.findChild<QPushButton *>("inspectCamera"));
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
