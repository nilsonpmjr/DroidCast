#include "window.h"
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QVBoxLayout>

PrototypeWindow::PrototypeWindow(bool noTray)
    : trayForcedUnavailable(noTray) {
  setWindowTitle("DroidCast — Protótipo");
  resize(640, 460);
  setMinimumSize(540, 400);
  auto *root = new QWidget;
  auto *layout = new QVBoxLayout(root);
  layout->setContentsMargins(24, 20, 24, 20);
  layout->setSpacing(16);
  auto *title = new QLabel("DroidCast");
  auto font = title->font();
  font.setPointSize(22);
  title->setFont(font);
  layout->addWidget(title);
  auto *disclaimer = new QLabel(
      "Protótipo interativo · dispositivos e sessões simulados\n"
      "Não espelha, não envia arquivos e não configura início com o sistema.");
  disclaimer->setWordWrap(true);
  layout->addWidget(disclaimer);

  auto *device = new QGroupBox("Dispositivo");
  auto *deviceLayout = new QVBoxLayout(device);
  deviceChoice = new QComboBox;
  deviceChoice->setObjectName("deviceChoice");
  deviceChoice->setAccessibleName("Dispositivo simulado");
  deviceChoice->addItems({"Pixel de demonstração", "Android secundário de demonstração"});
  deviceLayout->addWidget(deviceChoice);
  deviceStatus = new QLabel;
  deviceStatus->setObjectName("deviceStatus");
  deviceLayout->addWidget(deviceStatus);
  auto *row = new QHBoxLayout;
  mode = new QComboBox;
  mode->setObjectName("sessionMode");
  mode->setAccessibleName("Modo de sessão simulada");
  mode->addItems({"Espelhamento", "Câmera", "Somente áudio",
                  "Display virtual", "Controle USB / OTG"});
  row->addWidget(mode, 1);
  startButton = new QPushButton("Iniciar");
  startButton->setObjectName("startSession");
  row->addWidget(startButton);
  deviceLayout->addLayout(row);
  layout->addWidget(device);

  offer = new QGroupBox("Android conectado — Pixel de demonstração");
  offer->setObjectName("usbOffer");
  auto *offerLayout = new QVBoxLayout(offer);
  offerStatus = new QLabel;
  offerStatus->setWordWrap(true);
  offerLayout->addWidget(offerStatus);
  auto *offerRow = new QHBoxLayout;
  offerMirror = new QPushButton("Espelhar");
  offerMirror->setObjectName("offerMirror");
  offerTransfer = new QPushButton("Transferir arquivo…");
  offerTransfer->setObjectName("offerTransfer");
  auto *later = new QPushButton("Agora não");
  later->setObjectName("dismissOffer");
  for (auto *button : {offerMirror, offerTransfer, later})
    offerRow->addWidget(button);
  offerLayout->addLayout(offerRow);
  suppressOffers = new QCheckBox("Não oferecer ações nas próximas conexões");
  suppressOffers->setObjectName("suppressOffers");
  offerLayout->addWidget(suppressOffers);
  layout->addWidget(offer);
  offer->hide();

  auto *sessionRow = new QHBoxLayout;
  sessionStatus = new QLabel;
  sessionStatus->setObjectName("sessionStatus");
  sessionStatus->setWordWrap(true);
  sessionRow->addWidget(sessionStatus, 1);
  stopButton = new QPushButton("Encerrar sessão");
  stopButton->setObjectName("stopSession");
  sessionRow->addWidget(stopButton);
  layout->addLayout(sessionRow);
  layout->addStretch();
  setCentralWidget(root);

  auto *deviceMenu = menuBar()->addMenu("Dispositivo");
  auto *wifi = deviceMenu->addAction("Conectar por Wi-Fi…");
  connect(wifi, &QAction::triggered, this, [this] {
    QMessageBox::information(this, "Fluxo planejado", "Conexão Wi-Fi será um diálogo guiado na migração. Este protótipo testa a organização.");
  });
  transferAction = deviceMenu->addAction("Transferir arquivo…");
  transferAction->setObjectName("transferAction");
  connect(transferAction, &QAction::triggered, this, &PrototypeWindow::transferDemo);
  auto *sessionMenu = menuBar()->addMenu("Sessão");
  startAction = sessionMenu->addAction("Iniciar sessão simulada");
  startAction->setObjectName("startAction");
  startAction->setShortcut(QKeySequence("Ctrl+Return"));
  stopAction = sessionMenu->addAction("Encerrar sessão");
  stopAction->setObjectName("stopAction");
  auto *settings = menuBar()->addMenu("Ferramentas")->addAction("Configurações…");
  settings->setMenuRole(QAction::PreferencesRole);
  settings->setShortcut(QKeySequence::Preferences);
  connect(settings, &QAction::triggered, this, &PrototypeWindow::showSettings);
  auto *help = menuBar()->addMenu("Ajuda");
  auto *about = help->addAction("Sobre o protótipo");
  about->setMenuRole(QAction::AboutRole);
  connect(about, &QAction::triggered, this, [this] {
    QMessageBox::information(this, "DroidCast Prototype", "Experimento N01: interface nativa compacta.\nEstados simulados não provam USB, SDL, transferência ou hardware.");
  });
  auto *quit = help->addAction("Sair do protótipo");
  quit->setMenuRole(QAction::QuitRole);
  quit->setShortcut(QKeySequence::Quit);
  connect(quit, &QAction::triggered, qApp, &QApplication::quit);

  auto *demo = menuBar()->addMenu("Demonstração");
  auto addDemo = [this, demo](const QString &text, auto callback) {
    connect(demo->addAction(text), &QAction::triggered, this, callback);
  };
  addDemo("Conectar USB autorizado", [this] { simulateConnection(); });
  addDemo("Conectar USB sem autorização", [this] { simulateConnection(false); });
  addDemo("Autorizar dispositivo", [this] { simulateAuthorization(); });
  addDemo("Repetir evento de descoberta", [this] { simulateConnection(authorized); });
  addDemo("Desconectar USB", [this] { simulateDisconnect(); });
  trayAction = demo->addAction("Usar tray / menu bar");
  trayAction->setCheckable(true);
  connect(trayAction, &QAction::toggled, this, &PrototypeWindow::setTrayEnabled);
  auto *unavailable = demo->addAction("Simular tray indisponível");
  unavailable->setCheckable(true);
  unavailable->setChecked(noTray);
  connect(unavailable, &QAction::toggled, this, [this](bool value) {
    trayForcedUnavailable = value;
    setTrayEnabled(trayEnabled);
    if (!trayUsable()) restoreWindow();
  });

  tray = new QSystemTrayIcon(style()->standardIcon(QStyle::SP_ComputerIcon), this);
  auto *trayMenu = new QMenu(this);
  connect(trayMenu->addAction("Abrir DroidCast Prototype"), &QAction::triggered,
          this, &PrototypeWindow::restoreWindow);
  trayMenu->addAction(startAction);
  trayMenu->addAction(transferAction);
  trayMenu->addAction(stopAction);
  trayMenu->addSeparator();
  trayMenu->addAction(quit);
  tray->setContextMenu(trayMenu);
  connect(tray, &QSystemTrayIcon::activated, this,
          [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
              restoreWindow();
          });
  connect(tray, &QSystemTrayIcon::messageClicked, this, &PrototypeWindow::restoreWindow);
  connect(deviceChoice, &QComboBox::currentIndexChanged, this, [this] {
    // Changing the demo target invalidates its connection and any pending offer.
    simulateDisconnect();
  });
  connect(startButton, &QPushButton::clicked, this, &PrototypeWindow::startDemo);
  connect(startAction, &QAction::triggered, this, &PrototypeWindow::startDemo);
  connect(stopButton, &QPushButton::clicked, this, &PrototypeWindow::stopDemo);
  connect(stopAction, &QAction::triggered, this, &PrototypeWindow::stopDemo);
  connect(offerMirror, &QPushButton::clicked, this, [this] {
    if (!connected || !authorized || active) return;
    mode->setCurrentIndex(0);
    dismissOffer();
    startDemo();
  });
  connect(offerTransfer, &QPushButton::clicked, this, &PrototypeWindow::transferDemo);
  connect(later, &QPushButton::clicked, this, &PrototypeWindow::dismissOffer);
  connect(mode, &QComboBox::currentIndexChanged, this, [this] { updateState(); });
  updateState();
}

bool PrototypeWindow::offerVisible() const { return !offer->isHidden(); }
bool PrototypeWindow::trayUsable() const {
  return trayEnabled && !trayForcedUnavailable && QSystemTrayIcon::isSystemTrayAvailable();
}
void PrototypeWindow::setTrayEnabled(bool enabled) {
  trayEnabled = enabled;
  tray->setVisible(trayUsable());
  updateState();
}
void PrototypeWindow::simulateConnection(bool permitted) {
  const bool newConnection = !connected;
  connected = true;
  authorized = permitted;
  if (newConnection && offersEnabled && !active) {
    offer->show();
    if (trayUsable() && !isVisible())
      tray->showMessage("Android conectado (simulado)",
          "Abra DroidCast para escolher Espelhar ou Transferir arquivo…");
  }
  updateState();
}
void PrototypeWindow::simulateAuthorization() {
  if (!connected) return;
  authorized = true;
  updateState();
}
void PrototypeWindow::simulateDisconnect() {
  connected = authorized = active = false;
  offer->hide();
  updateState();
}
void PrototypeWindow::dismissOffer() {
  offersEnabled = !suppressOffers->isChecked();
  offer->hide();
}
void PrototypeWindow::startDemo() {
  if (!connected || active || (!authorized && mode->currentIndex() != 4)) return;
  active = true;
  activeMode = mode->currentText();
  dismissOffer();
  updateState();
}
void PrototypeWindow::stopDemo() { active = false; updateState(); }
void PrototypeWindow::updateState() {
  deviceStatus->setText(!connected ? "Nenhum dispositivo · use o menu Demonstração"
      : authorized ? "USB · Autorizado (simulado)"
                   : "USB · Autorize a depuração no Android (simulado)");
  offer->setTitle("Android conectado — " + deviceChoice->currentText());
  const bool canStart = connected && !active && (authorized || mode->currentIndex() == 4);
  startAction->setEnabled(canStart);
  startButton->setEnabled(canStart);
  stopAction->setEnabled(active);
  stopButton->setEnabled(active);
  mode->setEnabled(!active);
  deviceChoice->setEnabled(!active);
  transferAction->setEnabled(connected && authorized && !active);
  offerMirror->setEnabled(connected && authorized && !active);
  offerTransfer->setEnabled(transferAction->isEnabled());
  offerStatus->setText(authorized ? "O que deseja fazer? Nenhuma ação acontece sem sua escolha."
      : "Desbloqueie e autorize a depuração USB para espelhar ou enviar um arquivo. OTG fica no seletor de modo.");
  sessionStatus->setText(active ? "Sessão simulada · " + activeMode : "Nenhuma sessão ativa");
  tray->setToolTip("DroidCast Prototype — " + sessionStatus->text());
  statusBar()->showMessage(trayUsable() ? "Tray disponível · simulação sem processos Android"
      : "Modo janela · tray desabilitado ou indisponível");
}
void PrototypeWindow::restoreWindow() { showNormal(); raise(); activateWindow(); }
void PrototypeWindow::closeEvent(QCloseEvent *event) {
  if (background && trayUsable()) { event->ignore(); hide(); }
  else QMainWindow::closeEvent(event);
}
void PrototypeWindow::showSettings() {
  QDialog dialog(this);
  dialog.setWindowTitle("Configurações do protótipo");
  auto *layout = new QVBoxLayout(&dialog);
  auto *backgroundChoice = new QCheckBox("Manter disponível no tray ao fechar a janela");
  backgroundChoice->setChecked(background);
  layout->addWidget(backgroundChoice);
  auto *offerChoice = new QCheckBox("Oferecer ações ao conectar USB (simulado)");
  offerChoice->setChecked(offersEnabled);
  layout->addWidget(offerChoice);
  auto *login = new QCheckBox("Iniciar com o sistema — planejado, não configurado");
  login->setEnabled(false);
  layout->addWidget(login);
  auto *info = new QLabel("A aparência acompanha o sistema.\nPreferências deste experimento não são salvas.\nConfigurações avançadas serão preservadas na migração.");
  layout->addWidget(info);
  auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  if (dialog.exec() == QDialog::Accepted) {
    background = backgroundChoice->isChecked();
    offersEnabled = offerChoice->isChecked();
    suppressOffers->setChecked(!offersEnabled);
    if (!offersEnabled) offer->hide();
  }
}
void PrototypeWindow::transferDemo() {
  if (!connected || !authorized || active) return;
  const auto target = deviceChoice->currentText();
  dismissOffer();
  const auto file = QFileDialog::getOpenFileName(this, "Escolher arquivo — somente demonstração");
  if (file.isEmpty()) return;
  // File dialogs run a nested event loop: revalidate the simulated target.
  if (!connected || !authorized || active || deviceChoice->currentText() != target) return;
  QDialog dialog(this);
  dialog.setWindowTitle("Prévia de transferência — nenhum arquivo será enviado");
  auto *layout = new QVBoxLayout(&dialog);
  layout->addWidget(new QLabel("Dispositivo: " + target + "\nArquivo: " + QFileInfo(file).fileName()));
  layout->addWidget(new QLabel("Diretório Android (exemplo):"));
  layout->addWidget(new QLineEdit("/sdcard/Download/"));
  auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  dialog.exec();
}
