#include "window.h"
#include "ui.h"
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QUrl>
#include <QUuid>
#include <QVBoxLayout>
#include <algorithm>

namespace {
QLabel *label(const QString &text, const char *role = "body") {
  auto *l = new QLabel(text);
  l->setProperty("role", role);
  l->setWordWrap(true);
  l->setTextFormat(Qt::PlainText);
  if (QString::fromLatin1(role) == "badge") {
    l->setWordWrap(false);
    l->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  }
  l->setTextInteractionFlags(Qt::TextSelectableByMouse);
  return l;
}
QLabel *iconLabel(const QString &name, int size = 28) {
  auto *l = new QLabel;
  l->setPixmap(appIcon(name, QColor("#91bbff")).pixmap(size, size));
  l->setFixedSize(size + 12, size + 12);
  l->setAlignment(Qt::AlignCenter);
  return l;
}
QPushButton *button(const QString &text, const QString &icon = {},
                    const char *name = "") {
  auto *b = new QPushButton(text);
  b->setObjectName(name);
  b->setMinimumHeight(34);
  if (!icon.isEmpty())
    b->setIcon(appIcon(icon));
  return b;
}
QFrame *card(QVBoxLayout *&layout, const char *role = "card") {
  auto *frame = new QFrame;
  frame->setProperty("role", role);
  layout = new QVBoxLayout(frame);
  layout->setContentsMargins(20, 18, 20, 18);
  layout->setSpacing(12);
  return frame;
}
QWidget *page(const QString &title, const QString &subtitle,
              QVBoxLayout *&layout) {
  auto *widget = new QWidget;
  widget->setObjectName("workspace");
  widget->setMaximumWidth(1240);
  layout = new QVBoxLayout(widget);
  layout->setContentsMargins(28, 24, 28, 24);
  layout->setSpacing(18);
  layout->setAlignment(Qt::AlignTop);
  layout->addWidget(label(title, "title"));
  layout->addWidget(label(subtitle, "muted"));
  return widget;
}
void field(QFormLayout *form, const QString &text, QWidget *control) {
  auto *l = new QLabel(text);
  l->setBuddy(control);
  control->setAccessibleName(text);
  form->addRow(l, control);
}
QCheckBox *toggle(const QString &text, bool checked) {
  auto *check = new PreferenceSwitch(text);
  check->setChecked(checked);
  return check;
}
QString captureName(const QString &extension) {
  return "DroidCast-" +
         QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss-zzz") + "-" +
         QUuid::createUuid().toString(QUuid::Id128).left(6) + extension;
}
} // namespace

Window::Window(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("DroidCast Desktop");
  setWindowIcon(appIcon("brand", QColor("#91bbff")));
  resize(1240, 860);
  setMinimumSize(920, 680);
  engine.preferences = Preferences::load(settings);
  auto *root = new QWidget;
  auto *horizontal = new QHBoxLayout(root);
  horizontal->setContentsMargins(0, 0, 0, 0);
  horizontal->setSpacing(0);
  auto *sidebar = new QWidget;
  sidebar->setObjectName("sidebar");
  sidebar->setFixedWidth(238);
  auto *side = new QVBoxLayout(sidebar);
  side->setContentsMargins(16, 22, 16, 20);
  side->setSpacing(14);
  auto *brand = new QHBoxLayout;
  brand->addWidget(iconLabel("brand", 32));
  auto *brandText = new QVBoxLayout;
  brandText->setSpacing(2);
  brandText->addWidget(label("DroidCast", "brand"));
  brandText->addWidget(label("Desktop", "muted"));
  brand->addLayout(brandText);
  brand->addStretch();
  side->addLayout(brand);
  side->addSpacing(18);
  side->addWidget(label("Workspace", "muted"));
  navigation = new QListWidget;
  navigation->setObjectName("navigation");
  navigation->setAccessibleName("Main navigation");
  const QStringList names{"Connected devices", "Active session",
                          "Wireless pairing",  "Recordings & captures",
                          "Input & controls",  "Settings",
                          "Diagnostics"};
  const QStringList icons{"phone", "mirror",   "wifi",    "capture",
                          "input", "settings", "terminal"};
  for (int i = 0; i < names.size(); ++i)
    new QListWidgetItem(appIcon(icons[i]), names[i], navigation);
  side->addWidget(navigation, 1);
  QVBoxLayout *runtimeLayout;
  auto *runtime = card(runtimeLayout, "well");
  runtimeLayout->addWidget(label("DroidCast engine", "section"));
  runtimeStatus = label("Checking bundled services…", "muted");
  runtimeLayout->addWidget(runtimeStatus);
  side->addWidget(runtime);
  side->addWidget(label("Local connection. No account.", "muted"));
  horizontal->addWidget(sidebar);
  auto *main = new QWidget;
  auto *vertical = new QVBoxLayout(main);
  vertical->setContentsMargins(0, 0, 0, 0);
  vertical->setSpacing(0);
  auto *topbar = new QWidget;
  topbar->setObjectName("topbar");
  auto *top = new QHBoxLayout(topbar);
  top->setContentsMargins(28, 12, 28, 12);
  top->addWidget(label("Android workspace", "muted"));
  top->addStretch();
  headerStatus = label("No active session", "badge");
  top->addWidget(headerStatus);
  vertical->addWidget(topbar);
  pages = new QStackedWidget;
  auto *diagnostics = diagnosticsPage();
  for (auto *content :
       {devicesPage(), sessionPage(), wirelessPage(), capturesPage(),
        inputPage(), settingsPage(), diagnostics}) {
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(content);
    pages->addWidget(scroll);
  }
  vertical->addWidget(pages, 1);
  notice = label("Connect your phone to get started.");
  notice->setObjectName("notice");
  notice->setAccessibleName("Application status");
  notice->setMinimumHeight(48);
  vertical->addWidget(notice);
  horizontal->addWidget(main, 1);
  setCentralWidget(root);
  connect(navigation, &QListWidget::currentRowChanged, this, [this](int row) {
    pages->setCurrentIndex(row);
    if (row == 3)
      updateCaptures();
  });
  navigation->setCurrentRow(0);
  connect(&engine, &Engine::message, this, [this](const QString &text) {
    notice->setText(text);
    logs->appendPlainText(QDateTime::currentDateTime().toString("HH:mm:ss") +
                          "  " + text);
  });
  connect(&engine, &Engine::log, logs, &QPlainTextEdit::appendPlainText);
  connect(&engine, &Engine::devicesChanged, this, &Window::updateDevices);
  connect(&engine, &Engine::scanChanged, this, [this] {
    refreshButton->setEnabled(!engine.scanning());
    refreshButton->setText(engine.scanning() ? "Scanning…" : "Scan devices");
  });
  connect(&engine, &Engine::sessionChanged, this, [this] {
    if (engine.running() && !elapsed.isValid())
      elapsed.start();
    if (!engine.running()) {
      elapsed.invalidate();
      updateCaptures();
    }
    updateActions();
    if (closing && !engine.running())
      QTimer::singleShot(0, this, &QWidget::close);
  });
  connect(&engine, &Engine::deviceTaskChanged, this, &Window::updateActions);
  connect(&engine, &Engine::captureSaved, this, [this] { updateCaptures(); });
  connect(&poll, &QTimer::timeout, this, [this] {
    if (!closing && !executablePath(engine.preferences.adb).isEmpty())
      engine.refresh();
  });
  poll.start(4000);
  connect(&sessionClock, &QTimer::timeout, this, [this] {
    auto seconds = elapsed.isValid() ? elapsed.elapsed() / 1000 : 0;
    sessionTime->setText(QString("%1:%2")
                             .arg(seconds / 60, 2, 10, QChar('0'))
                             .arg(seconds % 60, 2, 10, QChar('0')));
  });
  sessionClock.start(1000);
  savePreferences();
  updateDevices();
  updateCaptures();
  QTimer::singleShot(0, &engine, &Engine::refresh);
}
void Window::showPage(int index) {
  if (index >= 0 && index < pages->count())
    navigation->setCurrentRow(index);
}

QWidget *Window::devicesPage() {
  QVBoxLayout *layout;
  auto *widget = page("Connected devices",
                      "Your Android devices, ready for a bigger screen. Choose "
                      "a device to start a session.",
                      layout);
  auto *actions = new QHBoxLayout;
  deviceCount = label("0 devices connected", "badge");
  actions->addWidget(deviceCount);
  actions->addStretch();
  refreshButton = button("Scan devices", "refresh", "refreshDevices");
  actions->addWidget(refreshButton);
  auto *wireless = button("Pair wirelessly", "wifi");
  wireless->setProperty("primary", true);
  actions->addWidget(wireless);
  layout->addLayout(actions);
  connect(refreshButton, &QPushButton::clicked, &engine, &Engine::refresh);
  connect(wireless, &QPushButton::clicked, this, [this] { showPage(2); });
  QVBoxLayout *welcomeLayout;
  welcome = card(welcomeLayout);
  auto *intro = new QHBoxLayout;
  intro->setSpacing(20);
  intro->addWidget(new ConnectionArt, 2);
  auto *instructions = new QVBoxLayout;
  instructions->addWidget(label("Welcome to DroidCast Desktop", "section"));
  instructions->addWidget(label("Bring your phone to your desktop. Mirror, "
                                "control and capture from one workspace.",
                                "muted"));
  instructions->addSpacing(8);
  instructions->addWidget(
      label("1   Enable USB debugging in Developer options."));
  instructions->addWidget(
      label("2   Connect your phone with a USB data cable."));
  instructions->addWidget(
      label("3   Unlock it and allow the debugging request."));
  intro->addLayout(instructions, 3);
  welcomeLayout->addLayout(intro);
  layout->addWidget(welcome);
  deviceList = new QListWidget;
  deviceList->setObjectName("devices");
  deviceList->setAccessibleName("Connected Android devices");
  deviceList->setMinimumHeight(170);
  layout->addWidget(deviceList, 1);
  deviceHelp = label("", "muted");
  layout->addWidget(deviceHelp);
  QVBoxLayout *quick;
  auto *quickCard = card(quick);
  auto *quickHeader = new QHBoxLayout;
  quickHeader->addWidget(iconLabel("settings", 20));
  quickHeader->addWidget(label("Session preferences", "section"));
  quickHeader->addStretch();
  auto *allSettings = button("All settings", "settings");
  connect(allSettings, &QPushButton::clicked, this, [this] { showPage(5); });
  quickHeader->addWidget(allSettings);
  quick->addLayout(quickHeader);
  auto *flags = new QHBoxLayout;
  for (auto pair :
       {qMakePair(QString("Forward audio"), &Preferences::audio),
        qMakePair(QString("Stay awake while charging"), &Preferences::awake),
        qMakePair(QString("Turn phone screen off"), &Preferences::screenOff)}) {
    auto *check = toggle(pair.first, engine.preferences.*pair.second);
    flags->addWidget(check);
    check->setProperty("preference", pair.first);
    connect(check, &QCheckBox::toggled, this,
            [this, member = pair.second](bool value) {
              engine.preferences.*member = value;
              savePreferences();
            });
  }
  quick->addLayout(flags);
  quick->addWidget(
      label("Preferences apply when a new session starts.", "muted"));
  layout->addWidget(quickCard);
  sessionStatus = label("No active session", "muted");
  layout->addWidget(sessionStatus);
  auto *sessionActions = new QHBoxLayout;
  startButton = button("Start mirroring", "play", "startMirror");
  startButton->setProperty("primary", true);
  recordButton = button("Mirror && record", "video", "recordMirror");
  stopButton = button("Stop session", "stop", "stopMirror");
  sessionActions->addWidget(startButton);
  sessionActions->addWidget(recordButton);
  sessionActions->addStretch();
  sessionActions->addWidget(stopButton);
  layout->addLayout(sessionActions);
  connect(startButton, &QPushButton::clicked, this, [this] { launch(false); });
  connect(recordButton, &QPushButton::clicked, this, [this] { launch(true); });
  connect(stopButton, &QPushButton::clicked, &engine, &Engine::stop);
  connect(deviceList, &QListWidget::currentRowChanged, this,
          &Window::updateActions);
  return widget;
}
QWidget *Window::sessionPage() {
  QVBoxLayout *layout;
  auto *widget = page(
      "Active session",
      "Your mirroring session and device tools, together in one workspace.",
      layout);
  auto *columns = new QHBoxLayout;
  columns->setSpacing(18);
  QVBoxLayout *preview;
  auto *previewCard = card(preview, "well");
  preview->addStretch();
  preview->addWidget(new ConnectionArt);
  sessionDetail = label(
      "No mirror running\n\nChoose a connected device and start mirroring.");
  sessionDetail->setAlignment(Qt::AlignCenter);
  preview->addWidget(sessionDetail);
  auto *choose = button("Choose a device", "phone");
  connect(choose, &QPushButton::clicked, this, [this] { showPage(0); });
  preview->addWidget(choose);
  auto *phoneControls = new QGridLayout;
  int actionIndex = 0;
  for (const auto &entry :
       {qMakePair(QString("Back"), Engine::PhoneAction::Back),
        qMakePair(QString("Home"), Engine::PhoneAction::Home),
        qMakePair(QString("Recent apps"), Engine::PhoneAction::Recents),
        qMakePair(QString("Power"), Engine::PhoneAction::Power),
        qMakePair(QString("Volume −"), Engine::PhoneAction::VolumeDown),
        qMakePair(QString("Volume +"), Engine::PhoneAction::VolumeUp)}) {
    auto *control = button(entry.first);
    control->setProperty("phoneControl", true);
    control->setAccessibleName("Phone: " + entry.first);
    control->setToolTip(
        "Send " + entry.first +
        " to the active phone, or the selected device when idle.");
    phoneControls->addWidget(control, actionIndex / 3, actionIndex % 3);
    ++actionIndex;
    connect(control, &QPushButton::clicked, this,
            [this, action = entry.second] {
              engine.phoneAction(engine.running() ? engine.activeSerial
                                                  : selectedSerial(),
                                 action);
            });
  }
  preview->addLayout(phoneControls);
  auto *cameraPanel = new QWidget;
  cameraPanel->setObjectName("cameraControlsPanel");
  auto *cameraLayout = new QVBoxLayout(cameraPanel);
  cameraLayout->setContentsMargins(0, 6, 0, 0);
  cameraLayout->setSpacing(8);
  cameraLayout->addWidget(label("Camera controls", "section"));
  cameraLayout->addWidget(
      label("Control torch and zoom on the active phone camera. Availability "
            "depends on the camera.",
            "muted"));
  auto *cameraControls = new QGridLayout;
  cameraControls->setSpacing(8);
  int cameraIndex = 0;
  for (const auto &entry :
       {qMakePair(QString("Torch on"), Engine::CameraAction::TorchOn),
        qMakePair(QString("Torch off"), Engine::CameraAction::TorchOff),
        qMakePair(QString("Zoom out"), Engine::CameraAction::ZoomOut),
        qMakePair(QString("Zoom in"), Engine::CameraAction::ZoomIn)}) {
    auto *control = button(entry.first);
    control->setProperty("cameraControl", true);
    control->setAccessibleName("Camera: " + entry.first);
    control->setToolTip(entry.first + " on the active phone camera.");
    control->setMinimumHeight(40);
    cameraControls->addWidget(control, cameraIndex / 2, cameraIndex % 2);
    ++cameraIndex;
    connect(control, &QPushButton::clicked, this,
            [this, action = entry.second] { engine.cameraAction(action); });
  }
  cameraLayout->addLayout(cameraControls);
  auto *cameraFeedback =
      label("Start a camera session to use these controls.", "muted");
  cameraFeedback->setObjectName("cameraControlFeedback");
  cameraLayout->addWidget(cameraFeedback);
  cameraPanel->hide();
  preview->addWidget(cameraPanel);
  connect(&engine, &Engine::cameraControlMessage, cameraFeedback,
          &QLabel::setText);
  connect(&engine, &Engine::cameraControlsChanged, this,
          &Window::updateActions);
  connect(&engine, &Engine::sessionChanged, this, [this, cameraFeedback] {
    if (!engine.cameraSession())
      cameraFeedback->setText("Start a camera session to use these controls.");
  });
  preview->addWidget(label("Mirror window", "section"));
  preview->addWidget(
      label("Window presentation controls. In resizable virtual-display mode, "
            "resizing also changes Android's display. Pausing the image does "
            "not pause audio or recording.",
            "muted"));
  auto *windowControls = new QGridLayout;
  windowControls->setSpacing(8);
  int windowIndex = 0;
  for (const auto &entry :
       {qMakePair(QString("Toggle fullscreen"),
                  Engine::WindowAction::Fullscreen),
        qMakePair(QString("Fit window"), Engine::WindowAction::Fit),
        qMakePair(QString("Pixel-perfect size"),
                  Engine::WindowAction::PixelPerfect),
        qMakePair(QString("Rotate left"), Engine::WindowAction::RotateLeft),
        qMakePair(QString("Rotate right"), Engine::WindowAction::RotateRight),
        qMakePair(QString("Pause image"), Engine::WindowAction::Pause),
        qMakePair(QString("Resume image"), Engine::WindowAction::Resume)}) {
    auto *control = button(entry.first);
    control->setProperty("windowControl", true);
    control->setAccessibleName("Mirror window: " + entry.first);
    control->setMinimumHeight(40);
    if (windowIndex == 0)
      windowControls->addWidget(control, 0, 0, 1, 2);
    else
      windowControls->addWidget(control, (windowIndex + 1) / 2,
                                (windowIndex - 1) % 2);
    ++windowIndex;
    connect(control, &QPushButton::clicked, this,
            [this, action = entry.second] { engine.windowAction(action); });
  }
  preview->addLayout(windowControls);
  auto *windowFeedback =
      label("Start mirroring to use window controls.", "muted");
  windowFeedback->setObjectName("windowControlFeedback");
  preview->addWidget(windowFeedback);
  connect(&engine, &Engine::windowControlMessage, windowFeedback,
          &QLabel::setText);
  connect(&engine, &Engine::windowControlsChanged, this,
          &Window::updateActions);
  connect(&engine, &Engine::sessionChanged, this, [this, windowFeedback] {
    if (engine.sessionState() != Engine::SessionState::Streaming)
      windowFeedback->setText(engine.sessionStateText());
  });
  preview->addStretch();
  columns->addWidget(previewCard, 3);
  auto *controls = new QVBoxLayout;
  QVBoxLayout *session;
  auto *sessionCard = card(session);
  session->addWidget(label("Session controls", "section"));
  sessionTime = label("00:00", "title");
  sessionTime->setAccessibleName("Elapsed process time");
  session->addWidget(sessionTime);
  screenshotButton = button("Take screenshot", "capture");
  session->addWidget(screenshotButton);
  connect(screenshotButton, &QPushButton::clicked, this,
          &Window::captureScreen);
  sessionStop = button("Stop session", "stop");
  sessionStop->setProperty("danger", true);
  session->addWidget(sessionStop);
  connect(sessionStop, &QPushButton::clicked, &engine, &Engine::stop);
  controls->addWidget(sessionCard);
  QVBoxLayout *install;
  auto *installCard = card(install);
  install->addWidget(label("Install application", "section"));
  install->addWidget(label("Choose an APK to install on the selected phone. "
                           "Your phone may ask you to allow installation.",
                           "muted"));
  apkButton = button("Choose APK…", "folder");
  install->addWidget(apkButton);
  connect(apkButton, &QPushButton::clicked, this, [this] {
    const auto serial =
        engine.running() ? engine.activeSerial : selectedSerial();
    const auto path = QFileDialog::getOpenFileName(
        this, "Install Android application", {}, "Android packages (*.apk)");
    if (!path.isEmpty())
      engine.installApk(serial, path);
  });
  controls->addWidget(installCard);
  QVBoxLayout *keys;
  auto *keysCard = card(keys);
  keys->addWidget(label("Mirror shortcuts", "section"));
  keys->addWidget(
      label("Alt + F       Fullscreen\nAlt + H       Android Home\nAlt + B     "
            "  Android Back\nAlt + O       Turn screen off",
            "muted"));
  controls->addWidget(keysCard);
  controls->addStretch();
  columns->addLayout(controls, 2);
  layout->addLayout(columns, 1);
  layout->addWidget(label("The live display opens in its own DroidCast window. "
                          "Close that window to finish recordings cleanly.",
                          "muted"));
  return widget;
}
QWidget *Window::wirelessPage() {
  QVBoxLayout *layout;
  auto *widget = page("Wireless pairing",
                      "Connect without a cable. Keep your Android phone and "
                      "computer on the same trusted Wi-Fi network.",
                      layout);
  QVBoxLayout *help;
  auto *helpCard = card(help);
  help->addWidget(label("Before you connect", "section"));
  help->addWidget(label(
      "On Android 11 or newer, enable Developer options → Wireless debugging. "
      "Pair this computer once, then connect using the address on your phone.",
      "muted"));
  layout->addWidget(helpCard);
  auto *pairBox = new QGroupBox("1   Pair your phone");
  auto *pairForm = new QFormLayout(pairBox);
  pairForm->setSpacing(14);
  pairForm->addRow(
      label("Tap “Pair device with pairing code” on your phone.", "muted"));
  auto *pairAddress = new QLineEdit;
  pairAddress->setPlaceholderText("192.168.1.42:37123");
  field(pairForm, "Pairing address", pairAddress);
  auto *code = new QLineEdit;
  code->setEchoMode(QLineEdit::Password);
  code->setMaxLength(6);
  field(pairForm, "Six-digit code", code);
  auto *pairButton = button("Pair computer", "wifi");
  pairForm->addRow(pairButton);
  layout->addWidget(pairBox);
  auto *connectBox = new QGroupBox("2   Connect the paired phone");
  auto *connectForm = new QFormLayout(connectBox);
  connectForm->setSpacing(14);
  connectForm->addRow(
      label("Use the address on the main Wireless debugging screen. Its port "
            "usually differs from the pairing port.",
            "muted"));
  auto *address = new QLineEdit;
  address->setPlaceholderText("192.168.1.42:39517");
  field(connectForm, "Connection address", address);
  auto *connectButton = button("Connect phone", "phone");
  connectButton->setProperty("primary", true);
  connectForm->addRow(connectButton);
  layout->addWidget(connectBox);
  auto update = [this, pairAddress, address, code, pairButton, connectButton] {
    pairButton->setEnabled(
        !engine.wirelessBusy() &&
        validEndpoint(pairAddress->text().trimmed()) &&
        QRegularExpression("\\A[0-9]{6}\\z").match(code->text()).hasMatch());
    connectButton->setEnabled(!engine.wirelessBusy() &&
                              validEndpoint(address->text().trimmed()));
  };
  connect(pairAddress, &QLineEdit::textChanged, this, update);
  connect(address, &QLineEdit::textChanged, this, update);
  connect(code, &QLineEdit::textChanged, this, update);
  connect(&engine, &Engine::wirelessChanged, this, update);
  connect(pairButton, &QPushButton::clicked, this, [this, pairAddress, code] {
    engine.connectWireless(pairAddress->text().trimmed(), code->text());
    code->clear();
  });
  connect(connectButton, &QPushButton::clicked, this, [this, address] {
    engine.connectWireless(address->text().trimmed());
  });
  update();
  layout->addWidget(
      label("Already paired? Go straight to step 2. Older Android versions "
            "require TCP/IP debugging enabled over USB first.",
            "muted"));
  layout->addStretch();
  return widget;
}
QWidget *Window::capturesPage() {
  QVBoxLayout *layout;
  auto *widget = page("Recordings & captures",
                      "Screenshots and recordings saved on this computer. Open "
                      "a file in your preferred viewer.",
                      layout);
  auto *actions = new QHBoxLayout;
  captureFilter = new QComboBox;
  captureFilter->addItems({"All captures", "Videos", "Screenshots"});
  captureFilter->setAccessibleName("Filter captures");
  actions->addWidget(captureFilter);
  actions->addStretch();
  auto *refresh = button("Refresh", "refresh");
  auto *folder = button("Open folder", "folder");
  actions->addWidget(refresh);
  actions->addWidget(folder);
  layout->addLayout(actions);
  connect(captureFilter, &QComboBox::currentIndexChanged, this,
          &Window::updateCaptures);
  connect(refresh, &QPushButton::clicked, this, &Window::updateCaptures);
  connect(folder, &QPushButton::clicked, this, [this] {
    if (ensureCaptureDirectory() &&
        !QDesktopServices::openUrl(
            QUrl::fromLocalFile(engine.preferences.mediaDirectory)))
      notice->setText("Could not open the capture folder.");
  });
  captureDirectory = label(engine.preferences.mediaDirectory, "muted");
  layout->addWidget(captureDirectory);
  captureList = new QListWidget;
  captureList->setAccessibleName("Saved recordings and screenshots");
  captureList->setMinimumHeight(260);
  layout->addWidget(captureList, 1);
  auto openCapture = [this] {
    auto *item = captureList->currentItem();
    if (!item)
      return;
    const auto path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty() && path == engine.activeRecording) {
      notice->setText("Finish the recording before opening it.");
      return;
    }
    if (!path.isEmpty() &&
        !QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
      notice->setText("No application could open this capture.");
  };
  connect(captureList, &QListWidget::itemDoubleClicked, this, openCapture);
  auto *open = button("Open selected capture", "play");
  layout->addWidget(open);
  connect(open, &QPushButton::clicked, this, openCapture);
  return widget;
}
QWidget *Window::inputPage() {
  QVBoxLayout *layout;
  auto *widget = page("Input & controls",
                      "Choose how your keyboard and mouse interact with "
                      "Android. Changes apply to the next session.",
                      layout);
  for (auto pair :
       {qMakePair(QString("Keyboard simulation"), &Preferences::keyboard),
        qMakePair(QString("Mouse simulation"), &Preferences::mouse)}) {
    QVBoxLayout *content;
    auto *box = card(content);
    content->addWidget(label(pair.first, "section"));
    content->addWidget(
        label("Standard mode uses Android input events. Physical mode uses "
              "UHID to simulate hardware attached to the phone.",
              "muted"));
    auto *mode = new QComboBox;
    mode->setAccessibleName(pair.first);
    mode->addItem("Standard · SDK", "sdk");
    mode->addItem("Physical device · UHID", "uhid");
    mode->addItem("Disabled", "disabled");
    mode->setCurrentIndex(mode->findData(engine.preferences.*pair.second));
    content->addWidget(mode);
    connect(mode, &QComboBox::currentIndexChanged, this,
            [this, mode, member = pair.second] {
              engine.preferences.*member = mode->currentData().toString();
              savePreferences();
            });
    layout->addWidget(box);
  }
  auto *advancedInput =
      button("Advanced keyboard, mouse and gamepad settings", "settings");
  connect(advancedInput, &QPushButton::clicked, this, [this] {
    findChild<QLineEdit *>("settingsSearch")->clear();
    findChild<QComboBox *>("settingsCategory")->setCurrentText("Keyboard");
    showPage(5);
  });
  layout->addWidget(advancedInput);
  QVBoxLayout *keys;
  auto *keysCard = card(keys);
  keys->addWidget(label("Keyboard shortcuts", "section"));
  keys->addWidget(label(
      "In the mirror window, hold left Alt or left Super (Windows / Command) "
      "with the keys below. Ctrl combinations are sent to your Android app.",
      "muted"));
  auto *form = new QFormLayout;
  form->setSpacing(12);
  for (auto pair :
       {qMakePair("Fullscreen", "F / F11"),
        qMakePair("Android Home / Back", "H / B"),
        qMakePair("Recent apps", "S"),
        qMakePair("Turn phone screen off / on", "O / Shift + O"),
        qMakePair("Notifications", "N"), qMakePair("Paste clipboard", "V")})
    form->addRow(label(pair.first), label(pair.second, "badge"));
  keys->addLayout(form);
  layout->addWidget(keysCard);
  layout->addStretch();
  return widget;
}
QWidget *Window::settingsPage() {
  QVBoxLayout *layout;
  auto *widget = page("Settings",
                      "Tune your next session. DroidCast manages its mirroring "
                      "engine and connection services for you.",
                      layout);
  auto *search = new QLineEdit;
  search->setObjectName("settingsSearch");
  search->setAccessibleName("Search settings");
  search->setPlaceholderText("Search settings, categories or scrcpy flags…");
  layout->addWidget(search);
  auto *categoryPicker = new QComboBox;
  categoryPicker->setObjectName("settingsCategory");
  categoryPicker->setAccessibleName("Configuration category");
  categoryPicker->addItems({"All settings", "Video", "Camera", "Audio",
                            "Device", "Window", "Keyboard", "Mouse", "Gamepad",
                            "Recording", "Virtual display"});
  layout->addWidget(categoryPicker);
  auto *routes = new QHBoxLayout;
  auto *connectionRoute = button("Connection settings", "wifi");
  auto *inputRoute = button("Keyboard, mouse && shortcuts", "input");
  routes->addWidget(connectionRoute);
  routes->addWidget(inputRoute);
  routes->addStretch();
  layout->addLayout(routes);
  connect(connectionRoute, &QPushButton::clicked, this,
          [this] { showPage(2); });
  connect(inputRoute, &QPushButton::clicked, this, [this] { showPage(4); });
  auto *video = new QGroupBox("Display && video quality");
  auto *form = new QFormLayout(video);
  form->setSpacing(14);
  form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
  auto *size = new QComboBox;
  size->setObjectName("base-max-size");
  size->setAccessibleName("Video resolution limit");
  size->addItem("Original resolution", 0);
  for (int v : {720, 1080, 1440, 1920})
    size->addItem(QString::number(v) + " px maximum dimension", v);
  int sizeIndex = size->findData(engine.preferences.size);
  if (sizeIndex < 0) {
    size->addItem(QString::number(engine.preferences.size) + " px",
                  engine.preferences.size);
    sizeIndex = size->count() - 1;
  }
  size->setCurrentIndex(sizeIndex);
  field(form, "Resolution limit", size);
  connect(size, &QComboBox::currentIndexChanged, this, [this, size] {
    engine.preferences.size = size->currentData().toInt();
    savePreferences();
  });
  auto *fps = new QSpinBox;
  fps->setButtonSymbols(QAbstractSpinBox::PlusMinus);
  fps->setRange(1, 240);
  fps->setSuffix(" fps");
  fps->setValue(engine.preferences.fps);
  field(form, "Frame rate limit", fps);
  connect(fps, &QSpinBox::valueChanged, this, [this](int v) {
    engine.preferences.fps = v;
    savePreferences();
  });
  auto *bitrate = new QSpinBox;
  bitrate->setButtonSymbols(QAbstractSpinBox::PlusMinus);
  bitrate->setRange(1, 200);
  bitrate->setSuffix(" Mbps");
  bitrate->setValue(engine.preferences.bitrate);
  field(form, "Video bitrate", bitrate);
  connect(bitrate, &QSpinBox::valueChanged, this, [this](int v) {
    engine.preferences.bitrate = v;
    savePreferences();
  });
  auto *codec = new QComboBox;
  codec->addItems({"h264", "h265", "av1"});
  codec->setCurrentText(engine.preferences.codec);
  field(form, "Video codec", codec);
  connect(codec, &QComboBox::currentTextChanged, this,
          [this](const QString &v) {
            engine.preferences.codec = v;
            savePreferences();
          });
  form->addRow(label("H.264 is the most compatible option. HEVC and AV1 "
                     "require support from your phone's encoder.",
                     "muted"));
  layout->addWidget(video);
  auto *behavior = new QGroupBox("Audio && device behavior");
  auto *flags = new QVBoxLayout(behavior);
  for (auto pair :
       {qMakePair(QString("Forward audio"), &Preferences::audio),
        qMakePair(QString("Stay awake while charging"), &Preferences::awake),
        qMakePair(QString("Turn phone screen off"), &Preferences::screenOff),
        qMakePair(QString("Keep mirror on top"), &Preferences::top)}) {
    auto *check = toggle(pair.first, engine.preferences.*pair.second);
    check->setProperty("preference", pair.first);
    flags->addWidget(check);
    connect(check, &QCheckBox::toggled, this,
            [this, member = pair.second](bool v) {
              engine.preferences.*member = v;
              savePreferences();
            });
  }
  flags->addWidget(label("Audio forwarding requires Android 11 or newer. Stay "
                         "awake applies while the phone is charging.",
                         "muted"));
  layout->addWidget(behavior);
  QList<QWidget *> searchable{video, behavior};
  video->setProperty("categories", QStringList{"Video"});
  behavior->setProperty("categories", QStringList{"Audio", "Device", "Window"});
  video->setProperty("searchTerms",
                     "video resolution fps frame rate bitrate codec");
  behavior->setProperty("searchTerms",
                        "audio device behavior awake screen top");
  for (const auto &category :
       {QString("Video"), QString("Camera"), QString("Audio"),
        QString("Device"), QString("Window"), QString("Keyboard"),
        QString("Mouse"), QString("Gamepad"), QString("Recording"),
        QString("Virtual display")}) {
    auto *group = new QGroupBox(category + " · advanced");
    auto *fields = new QFormLayout(group);
    fields->setSpacing(12);
    QString terms = category;
    if (category == "Camera") {
      cameraIdSelector = new QComboBox;
      cameraIdSelector->setObjectName("detectedCamera");
      cameraIdSelector->setAccessibleName("Detected camera");
      cameraSizeSelector = new QComboBox;
      cameraSizeSelector->setObjectName("detectedCameraSize");
      cameraSizeSelector->setAccessibleName("Declared camera size");
      cameraFpsSelector = new QComboBox;
      cameraFpsSelector->setObjectName("detectedCameraFps");
      cameraFpsSelector->setAccessibleName("Declared camera frame rate");
      field(fields, "Detected camera", cameraIdSelector);
      field(fields, "Declared size", cameraSizeSelector);
      field(fields, "Declared frame rate", cameraFpsSelector);
      fields->addRow(label(
          "Inspect the selected phone to populate these choices. Manual fields "
          "remain available because Android declarations may be incomplete.",
          "muted"));
      connect(cameraIdSelector, qOverload<int>(&QComboBox::activated), this,
              [this](int index) {
                const auto id = cameraIdSelector->itemData(index).toString();
                if (!id.isEmpty()) {
                  findChild<QLineEdit *>("option-camera-id")->setText(id);
                  updateCameraSelectors();
                }
              });
      connect(cameraSizeSelector, qOverload<int>(&QComboBox::activated), this,
              [this](int index) {
                const auto size =
                    cameraSizeSelector->itemData(index).toString();
                if (!size.isEmpty()) {
                  findChild<QLineEdit *>("option-camera-size")->setText(size);
                  updateCameraSelectors();
                }
              });
      connect(cameraFpsSelector, qOverload<int>(&QComboBox::activated), this,
              [this](int index) {
                const int fps = cameraFpsSelector->itemData(index).toInt();
                if (fps > 0)
                  findChild<QSpinBox *>("option-camera-fps")->setValue(fps);
              });
    }
    for (const auto &option : sessionOptions()) {
      if (option.category != category)
        continue;
      terms += " " + option.key + " " + option.title + " " + option.help;
      if (option.key == "key-injection")
        terms += " --prefer-text --raw-key-events";
      if (option.key == "virtual-display")
        terms += " --new-display";
      const auto current = normalizedOption(
          option, engine.preferences.options.value(option.key));
      QWidget *control;
      QLabel *validation = nullptr;
      if (!option.choices.isEmpty()) {
        auto *combo = new QComboBox;
        combo->addItems(option.choices);
        combo->setCurrentText(current.toString());
        connect(combo, &QComboBox::currentTextChanged, this,
                [this, key = option.key](const QString &value) {
                  engine.preferences.options.insert(key, value);
                  savePreferences();
                  if (key == "video-source" || key == "camera-facing")
                    updateCameraSelectors();
                });
        control = combo;
      } else if (option.initial.metaType().id() == QMetaType::QString) {
        auto *edit = new QLineEdit;
        edit->setMaxLength(256);
        edit->setText(current.toString());
        validation = label("", "muted");
        validation->setObjectName("error-" + option.key);
        auto validate = [edit, validation, key = option.key] {
          const auto error = textOptionError(key, edit->text().trimmed());
          validation->setText(error.isEmpty() ? QString{}
                                              : "Invalid value: " + error);
          validation->setVisible(edit->isEnabled() && !error.isEmpty());
          edit->setAccessibleDescription(error);
        };
        validate();
        connect(edit, &QLineEdit::editingFinished, this, validate);
        if (option.key == "camera-id" || option.key == "camera-size")
          connect(edit, &QLineEdit::editingFinished, this,
                  &Window::updateCameraSelectors);
        connect(edit, &QLineEdit::textChanged, this,
                [this, key = option.key](const QString &value) {
                  engine.preferences.options.insert(key, value.trimmed());
                  savePreferences();
                });
        control = edit;
      } else if (option.initial.metaType().id() == QMetaType::Bool) {
        auto *check = toggle(option.title, current.toBool());
        connect(check, &QCheckBox::toggled, this,
                [this, key = option.key](bool value) {
                  engine.preferences.options.insert(key, value);
                  savePreferences();
                  updateActions();
                });
        if (option.key == "camera-high-speed")
          connect(check, &QCheckBox::toggled, this,
                  &Window::updateCameraSelectors);
        control = check;
      } else {
        auto *spin = new QSpinBox;
        spin->setRange(option.minimum, option.maximum);
        spin->setValue(current.toInt());
        connect(spin, &QSpinBox::valueChanged, this,
                [this, key = option.key](int value) {
                  engine.preferences.options.insert(key, value);
                  savePreferences();
                });
        control = spin;
      }
      control->setObjectName("option-" + option.key);
      control->setToolTip(
          option.help +
          (option.key == "key-injection" ? "\n--prefer-text / --raw-key-events"
           : option.key == "virtual-display" ? "\n--new-display"
                                             : "\n--" + option.key));
      if (qobject_cast<QCheckBox *>(control)) {
        control->setAccessibleName(option.title);
        fields->addRow(control);
      } else {
        field(fields, option.title, control);
      }
      if (validation)
        fields->addRow(validation);
      fields->addRow(label(option.help, "muted"));
    }
    if (category == "Video" || category == "Camera") {
      const bool cameraReport = category == "Camera";
      terms += cameraReport
                   ? " inspect camera sizes rates list-cameras "
                     "list-camera-sizes"
                   : " inspect resources displays encoders list-displays "
                     "list-encoders";
      auto *inspect = button(cameraReport ? "Inspect phone cameras"
                                          : "Inspect selected phone",
                             "phone");
      inspect->setProperty("inspectDevice", true);
      inspect->setObjectName(cameraReport ? "inspectCamera" : "inspectDevice");
      auto *cancel = button("Cancel inspection");
      cancel->setProperty("cancelInspection", true);
      auto *actions = new QHBoxLayout;
      actions->addWidget(inspect);
      actions->addWidget(cancel);
      fields->addRow(actions);
      auto *report = new QPlainTextEdit;
      report->setObjectName(cameraReport ? "cameraCapabilities"
                                         : "deviceCapabilities");
      report->setAccessibleName(cameraReport
                                    ? "Selected phone cameras report"
                                    : "Selected phone displays and encoders "
                                      "report");
      report->setReadOnly(true);
      report->setMaximumBlockCount(1000);
      report->setMinimumHeight(160);
      report->setPlainText(cameraReport
                               ? "Select an authorized Android 12+ phone, then "
                                 "inspect it while no mirror is running. Copy "
                                 "a camera ID, size and supported frame rate "
                                 "into the fields above. Android's declared "
                                 "combinations may be incomplete or inaccurate."
                               : "Select an authorized phone in Connected "
                                 "devices, then inspect it while no mirror is "
                                 "running. Copy a display ID or matching "
                                 "encoder name into the fields above. Listed "
                                 "resources may change after reconnecting.");
      fields->addRow(report);
      connect(inspect, &QPushButton::clicked, this,
              [this] { engine.inspectDevice(selectedSerial()); });
      connect(cancel, &QPushButton::clicked, &engine,
              &Engine::cancelInspection);
      connect(&engine, &Engine::inspectionResult, report,
              [this, report, cameraReport](const QString &serial,
                                           const QString &output) {
                report->setPlainText("Device: " + serial + "\n\n" + output);
                if (cameraReport)
                  setCameraCapabilities(serial, output);
              });
      connect(&engine, &Engine::inspectionChanged, this,
              &Window::updateActions);
    }
    group->setProperty("searchTerms", terms);
    group->setProperty("categories", QStringList{category});
    searchable.append(group);
    layout->addWidget(group);
  }
  updateCameraSelectors();
  auto *noResults = label("No matching settings in this release.", "muted");
  noResults->hide();
  layout->addWidget(noResults);
  QVBoxLayout *storage;
  auto *storageCard = card(storage);
  storage->addWidget(label("Capture storage", "section"));
  auto *directory = label(engine.preferences.mediaDirectory, "muted");
  storage->addWidget(directory);
  auto *browse = button("Change capture folder…", "folder");
  storage->addWidget(browse);
  connect(browse, &QPushButton::clicked, this, [this, directory] {
    const auto path = QFileDialog::getExistingDirectory(
        this, "Choose capture folder", engine.preferences.mediaDirectory);
    if (!path.isEmpty()) {
      engine.preferences.mediaDirectory = path;
      directory->setText(path);
      savePreferences();
      updateCaptures();
    }
  });
  layout->addWidget(storageCard);
  storageCard->setProperty("categories", QStringList{"Recording"});
  storageCard->setProperty("searchTerms",
                           "recording capture storage folder directory");
  searchable.append(storageCard);
  auto filter = [searchable, noResults, search, categoryPicker] {
    bool found = false;
    for (auto *group : searchable) {
      const bool categoryMatches = categoryPicker->currentIndex() == 0 ||
                                   group->property("categories")
                                       .toStringList()
                                       .contains(categoryPicker->currentText());
      const bool matches =
          categoryMatches &&
          group->property("searchTerms")
              .toString()
              .contains(search->text().trimmed(), Qt::CaseInsensitive);
      group->setVisible(matches);
      found |= matches;
    }
    noResults->setVisible(!found);
  };
  connect(search, &QLineEdit::textChanged, this, filter);
  connect(categoryPicker, &QComboBox::currentIndexChanged, this, filter);
  layout->addWidget(
      label("Saved automatically on this computer. The engine, Android server "
            "and connection tools are included with DroidCast Desktop.",
            "muted"));
  layout->addStretch();
  return widget;
}
QWidget *Window::diagnosticsPage() {
  QVBoxLayout *layout;
  auto *widget =
      page("Diagnostics",
           "Connection and mirroring output. Logs stay in memory and may "
           "include device identifiers and network addresses.",
           layout);
  logs = new QPlainTextEdit;
  logs->setObjectName("diagnostics");
  logs->setAccessibleName("Engine diagnostic output");
  logs->setReadOnly(true);
  logs->setMaximumBlockCount(2000);
  layout->addWidget(logs, 1);
  auto *clear = button("Clear log", "terminal");
  connect(clear, &QPushButton::clicked, logs, &QPlainTextEdit::clear);
  layout->addWidget(clear);
  return widget;
}
void Window::setCameraCapabilities(const QString &serial,
                                   const QString &report) {
  if (serial != selectedSerial()) {
    cameraCapabilitySerial.clear();
    cameraCapabilities.clear();
  } else {
    cameraCapabilities = parseCameraCapabilities(report);
    cameraCapabilitySerial = cameraCapabilities.isEmpty() ? QString{} : serial;
  }
  updateCameraSelectors();
}
void Window::updateCameraSelectors() {
  if (!cameraIdSelector || !cameraSizeSelector || !cameraFpsSelector)
    return;
  const QSignalBlocker idBlocker(cameraIdSelector);
  const QSignalBlocker sizeBlocker(cameraSizeSelector);
  const QSignalBlocker fpsBlocker(cameraFpsSelector);
  const auto selectedId =
      engine.preferences.options.value("camera-id").toString().trimmed();
  const auto selectedSize =
      engine.preferences.options.value("camera-size").toString().trimmed();
  const int selectedFps =
      engine.preferences.options.value("camera-fps", 0).toInt();
  cameraIdSelector->clear();
  cameraIdSelector->addItem(cameraCapabilities.isEmpty()
                                ? "Inspect phone to discover cameras"
                                : "Use manual camera ID field below",
                            QString{});
  const CameraCapability *capability = nullptr;
  for (const auto &camera : cameraCapabilities) {
    cameraIdSelector->addItem(camera.id + " · " + camera.facing + " · " +
                                  camera.sensorSize,
                              camera.id);
    if (camera.id == selectedId)
      capability = &camera;
  }
  if (!capability && selectedId.isEmpty() && !cameraCapabilities.isEmpty()) {
    const auto facing =
        engine.preferences.options.value("camera-facing", "auto").toString();
    for (const auto &camera : cameraCapabilities)
      if (facing == "auto" || camera.facing == facing) {
        capability = &camera;
        break;
      }
  }
  const int cameraIndex = cameraIdSelector->findData(selectedId);
  cameraIdSelector->setCurrentIndex(qMax(0, cameraIndex));

  cameraSizeSelector->clear();
  cameraSizeSelector->addItem(capability ? "Use manual size field below"
                                         : "Select a detected camera first",
                              QString{});
  QStringList sizes;
  if (capability) {
    const bool highSpeed =
        engine.preferences.options.value("camera-high-speed", false).toBool();
    sizes =
        highSpeed ? capability->highSpeedFrameRates.keys() : capability->sizes;
    for (const auto &size : sizes)
      cameraSizeSelector->addItem(size, size);
  }
  const int sizeIndex = cameraSizeSelector->findData(selectedSize);
  cameraSizeSelector->setCurrentIndex(qMax(0, sizeIndex));

  cameraFpsSelector->clear();
  cameraFpsSelector->addItem(capability ? "Use frame-rate field below"
                                        : "Select a detected camera first",
                             0);
  QStringList frameRates;
  if (capability &&
      engine.preferences.options.value("camera-high-speed", false).toBool()) {
    if (!selectedSize.isEmpty())
      frameRates = capability->highSpeedFrameRates.value(selectedSize);
    else
      for (const auto &rates : capability->highSpeedFrameRates)
        for (const auto &rate : rates)
          if (!frameRates.contains(rate))
            frameRates.append(rate);
  } else if (capability)
    frameRates = capability->frameRates;
  std::sort(
      frameRates.begin(), frameRates.end(),
      [](const QString &a, const QString &b) { return a.toInt() < b.toInt(); });
  for (const auto &rate : frameRates)
    cameraFpsSelector->addItem(rate + " fps", rate.toInt());
  const int fpsIndex = cameraFpsSelector->findData(selectedFps);
  cameraFpsSelector->setCurrentIndex(qMax(0, fpsIndex));
  const bool cameraMode =
      engine.preferences.options.value("video-source", "display").toString() ==
      "camera";
  const bool found = capability != nullptr;
  cameraIdSelector->setEnabled(cameraMode && !cameraCapabilities.isEmpty());
  cameraSizeSelector->setEnabled(cameraMode && found && !sizes.isEmpty());
  cameraFpsSelector->setEnabled(cameraMode && found && !frameRates.isEmpty());
}
void Window::savePreferences() {
  engine.preferences.save(settings);
  for (const auto &option : sessionOptions())
    if (auto *control = findChild<QWidget *>("option-" + option.key)) {
      const bool available = sessionOptionAvailable(option, engine.preferences);
      const bool changed = control->isEnabled() != available;
      control->setEnabled(available);
      if (!available)
        if (auto *error = findChild<QLabel *>("error-" + option.key))
          error->hide();
      if (changed && available)
        if (auto *edit = qobject_cast<QLineEdit *>(control))
          QMetaObject::invokeMethod(edit, "editingFinished",
                                    Qt::DirectConnection);
    }
  if (auto *size = findChild<QComboBox *>("base-max-size")) {
    const bool exactCameraSize =
        engine.preferences.options.value("video-source", "display")
                .toString() == "camera" &&
        !engine.preferences.options.value("camera-size")
             .toString()
             .trimmed()
             .isEmpty();
    size->setEnabled(!exactCameraSize);
    size->setToolTip(exactCameraSize
                         ? "Exact camera size replaces this resolution limit."
                         : "Maximum captured video dimension; zero keeps the "
                           "original resolution.");
  }
  for (auto *check : findChildren<QCheckBox *>()) {
    const auto key = check->property("preference").toString();
    if (key.isEmpty())
      continue;
    bool value = key == "Forward audio"               ? engine.preferences.audio
                 : key == "Stay awake while charging" ? engine.preferences.awake
                 : key == "Turn phone screen off" ? engine.preferences.screenOff
                                                  : engine.preferences.top;
    QSignalBlocker blocker(check);
    check->setChecked(value);
  }
  const bool ready = !executablePath(engine.preferences.adb).isEmpty() &&
                     !executablePath(engine.preferences.scrcpy).isEmpty() &&
                     QFileInfo::exists(engine.preferences.server);
  runtimeStatus->setText(ready ? "Bundled services ready\nscrcpy 4.1"
                               : "Bundle incomplete\nSee Diagnostics");
}
void Window::updateDevices() {
  QString currentFingerprint;
  for (const auto &d : engine.devices)
    currentFingerprint += d.serial + '\t' + d.state + '\t' + d.model + '\n';
  if (currentFingerprint != fingerprint ||
      deviceList->count() != engine.devices.size()) {
    const auto selected = selectedSerial();
    QSignalBlocker blocker(deviceList);
    deviceList->clear();
    for (const auto &d : engine.devices) {
      auto *item = new QListWidgetItem(deviceList);
      item->setData(Qt::UserRole, d.serial);
      item->setData(Qt::UserRole + 1, d.state);
      item->setData(Qt::AccessibleTextRole,
                    d.model + ", " + d.serial + ", " + d.state);
      item->setSizeHint(QSize(300, 142));
      QVBoxLayout *content;
      auto *deviceCard = card(content);
      auto *row = new QHBoxLayout;
      row->addWidget(
          iconLabel(d.connection == "Wireless" ? "wifi" : "phone", 32));
      auto *details = new QVBoxLayout;
      details->addWidget(label(d.model, "section"));
      details->addWidget(
          label(d.connection + " connection  ·  " + d.serial, "muted"));
      row->addLayout(details, 1);
      row->addWidget(label(d.ready() ? "Ready to mirror" : d.state, "badge"));
      content->addLayout(row);
      auto *actions = new QHBoxLayout;
      auto *choose = button("Select device", "phone");
      actions->addWidget(choose);
      actions->addStretch();
      auto *capture = button("Screenshot", "capture");
      capture->setProperty("captureSerial", d.serial);
      capture->setProperty("captureReady", d.ready());
      capture->setEnabled(d.ready());
      actions->addWidget(capture);
      content->addLayout(actions);
      connect(choose, &QPushButton::clicked, this,
              [this, item] { deviceList->setCurrentItem(item); });
      connect(capture, &QPushButton::clicked, this, [this, serial = d.serial] {
        if (!engine.deviceBusy() && ensureCaptureDirectory())
          engine.capture(serial, QDir(engine.preferences.mediaDirectory)
                                     .filePath(captureName(".png")));
      });
      deviceList->setItemWidget(item, deviceCard);
      if (d.serial == selected)
        deviceList->setCurrentItem(item);
    }
    if (!deviceList->currentItem() && deviceList->count())
      deviceList->setCurrentRow(0);
    fingerprint = currentFingerprint;
  }
  welcome->setVisible(engine.devices.isEmpty());
  deviceList->setVisible(!engine.devices.isEmpty());
  deviceCount->setText(QString::number(engine.devices.size()) +
                       (engine.devices.size() == 1 ? " device connected"
                                                   : " devices connected"));
  updateActions();
}
QString Window::selectedSerial() const {
  return deviceList->currentItem()
             ? deviceList->currentItem()->data(Qt::UserRole).toString()
             : QString{};
}
void Window::updateActions() {
  if (!cameraCapabilitySerial.isEmpty() &&
      cameraCapabilitySerial != selectedSerial()) {
    cameraCapabilitySerial.clear();
    cameraCapabilities.clear();
    updateCameraSelectors();
  }
  auto *item = deviceList->currentItem();
  const auto state = item ? item->data(Qt::UserRole + 1).toString() : QString{};
  const bool ready = state == "device";
  const bool running = engine.running();
  startButton->setEnabled(ready && !running && !engine.inspecting());
  recordButton->setEnabled(startButton->isEnabled());
  stopButton->setEnabled(running && engine.sessionState() !=
                                        Engine::SessionState::Stopping);
  sessionStop->setEnabled(stopButton->isEnabled());
  bool activeReady = false;
  for (const auto &d : engine.devices)
    if (d.serial == engine.activeSerial && d.ready())
      activeReady = true;
  const bool deviceAvailable =
      (running ? activeReady : ready) && !engine.deviceBusy();
  screenshotButton->setEnabled(
      deviceAvailable &&
      engine.captureAllowed(running ? engine.activeSerial : selectedSerial()));
  apkButton->setEnabled(deviceAvailable && engine.controlAllowed());
  for (auto *control : findChildren<QPushButton *>())
    if (control->property("phoneControl").toBool())
      control->setEnabled(screenshotButton->isEnabled() &&
                          engine.controlAllowed() &&
                          !engine.usesAlternateDisplay());
    else if (control->property("cameraControl").toBool())
      control->setEnabled(engine.cameraControlsAvailable());
    else if (control->property("windowControl").toBool())
      control->setEnabled(engine.windowControlsAvailable());
    else if (control->property("inspectDevice").toBool())
      control->setEnabled(ready && !running && !engine.deviceBusy());
    else if (control->property("cancelInspection").toBool())
      control->setEnabled(engine.inspecting());
    else if (control->property("captureSerial").isValid())
      control->setEnabled(
          control->property("captureReady").toBool() && !engine.deviceBusy() &&
          engine.captureAllowed(control->property("captureSerial").toString()));
  if (auto *cameraPanel = findChild<QWidget *>("cameraControlsPanel"))
    cameraPanel->setVisible(engine.cameraSession());
  if (state == "unauthorized")
    deviceHelp->setText("Unlock your phone and accept the USB debugging "
                        "prompt. Devices refresh automatically.");
  else if (state == "offline")
    deviceHelp->setText("This phone is offline. Reconnect its cable or connect "
                        "again over Wi-Fi.");
  else if (state == "no permissions")
    deviceHelp->setText("USB access is blocked. Your Linux account needs the "
                        "Android USB permissions / udev rules.");
  else if (ready)
    deviceHelp->setText("Selected: " + selectedSerial() +
                        ". Choose Start mirroring when you are ready.");
  else if (item)
    deviceHelp->setText("Boot this device into Android to mirror it.");
  else
    deviceHelp->setText(
        "Waiting for your first device. Discovery runs automatically; "
        "mirroring starts only when you choose it.");
  sessionStatus->setText(engine.sessionStateText() +
                         (running ? ": " + engine.activeSerial : QString{}));
  headerStatus->setText(engine.sessionStateText());
  sessionDetail->setText(
      running
          ? engine.sessionStateText() + " · " + engine.activeSerial +
                "\n\nYour live display opens in a separate DroidCast window. " +
                (engine.cameraSession()
                     ? "Camera capture: Android display commands and toolbar "
                       "screenshots are unavailable; use session recording "
                       "for the camera stream. Mirror-window controls remain "
                       "available."
                 : engine.usesAlternateDisplay()
                     ? "Secondary/virtual display: use input inside the "
                       "mirror. Phone-toolbar commands and screenshots are "
                       "unavailable for this display; recording remains "
                       "available. A virtual display without a launcher may "
                       "need a Start Android package setting to show content."
                     : "Use the controls here for screenshots and "
                       "applications.") +
                (engine.activeRecording.isEmpty()
                     ? QString{}
                     : "\n\nRecording to " +
                           QFileInfo(engine.activeRecording).fileName())
          : "No mirror running\n\nChoose a connected device and start "
            "mirroring.");
}
bool Window::ensureCaptureDirectory() {
  if (QDir().mkpath(engine.preferences.mediaDirectory))
    return true;
  notice->setText("Could not create the capture folder. Choose another folder "
                  "in Settings.");
  return false;
}
void Window::launch(bool record) {
  const auto issue = sessionIssue(engine.preferences, record);
  if (!issue.message.isEmpty()) {
    for (const auto &option : sessionOptions()) {
      if (option.key != issue.key)
        continue;
      findChild<QLineEdit *>("settingsSearch")->clear();
      findChild<QComboBox *>("settingsCategory")
          ->setCurrentText(option.category);
      showPage(5);
      if (auto *control = findChild<QWidget *>("option-" + option.key)) {
        control->setFocus();
        if (auto *edit = qobject_cast<QLineEdit *>(control))
          QMetaObject::invokeMethod(edit, "editingFinished",
                                    Qt::DirectConnection);
      }
      break;
    }
    notice->setText(issue.message);
    return;
  }
  const auto serial = selectedSerial();
  if (serial.isEmpty())
    return;
  QString path;
  if (record) {
    if (!ensureCaptureDirectory())
      return;
    path =
        QDir(engine.preferences.mediaDirectory)
            .filePath(captureName("." + recordingFormat(engine.preferences)));
  }
  if (engine.start(serial, path))
    showPage(1);
}
void Window::captureScreen() {
  if (engine.deviceBusy()) {
    notice->setText("Wait for the current device operation to finish.");
    return;
  }
  if (!ensureCaptureDirectory())
    return;
  engine.capture(
      engine.running() ? engine.activeSerial : selectedSerial(),
      QDir(engine.preferences.mediaDirectory).filePath(captureName(".png")));
}
void Window::updateCaptures() {
  const QString selected =
      captureList->currentItem()
          ? captureList->currentItem()->data(Qt::UserRole).toString()
          : QString{};
  captureList->clear();
  captureDirectory->setText(engine.preferences.mediaDirectory);
  const QStringList filters = captureFilter->currentIndex() == 1
                                  ? QStringList{"*.mkv", "*.mp4"}
                              : captureFilter->currentIndex() == 2
                                  ? QStringList{"*.png"}
                                  : QStringList{"*.mkv", "*.mp4", "*.png"};
  const auto files = QDir(engine.preferences.mediaDirectory)
                         .entryInfoList(filters, QDir::Files, QDir::Time);
  for (const auto &file : files) {
    const bool recording = file.absoluteFilePath() == engine.activeRecording;
    auto *item = new QListWidgetItem(
        appIcon(file.suffix() == "png" ? "capture" : "video"),
        file.fileName() + "\n" +
            file.lastModified().toString("dd MMM yyyy, HH:mm") + "   ·   " +
            QString::number(file.size() / 1024.0 / 1024.0, 'f', 2) + " MB" +
            (recording ? "   ·   Recording in progress" : ""),
        captureList);
    item->setData(Qt::UserRole, file.absoluteFilePath());
    if (file.absoluteFilePath() == selected)
      captureList->setCurrentItem(item);
  }
  if (files.isEmpty()) {
    auto *empty = new QListWidgetItem(
        "No captures yet\n\nTake a screenshot from a connected device, or "
        "choose Mirror & record to save your next session.",
        captureList);
    empty->setFlags(Qt::NoItemFlags);
  }
}
void Window::closeEvent(QCloseEvent *event) {
  if (engine.running()) {
    if (!closing && QMessageBox::question(
                        this, "End the active session?",
                        "Closing DroidCast Desktop stops its mirror. Close the "
                        "mirror window first to finish a recording cleanly.",
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::No) != QMessageBox::Yes) {
      event->ignore();
      return;
    }
    closing = true;
    poll.stop();
    engine.stop();
    event->ignore();
    return;
  }
  event->accept();
}
