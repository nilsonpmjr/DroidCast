#include "livewindow.h"
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QUuid>
#include <QVBoxLayout>

LiveWindow::LiveWindow(Engine *controller, bool discover,
                       std::function<bool()> available, QSettings *preferences)
    : engine(controller ? controller : new Engine(this)),
      trayAvailable(
          available ? std::move(available)
                    : [] { return QSystemTrayIcon::isSystemTrayAvailable(); }),
      uiPreferences(preferences ? preferences
                    : controller
                        ? nullptr
                        : new QSettings("DroidCast", "CompactPilot", this)) {
  if (!controller)
    engine->preferences = bundledPreferences();
  setWindowTitle("DroidCast — Compacto (experimental)");
  resize(640, 420);
  setMinimumSize(540, 360);
  auto *root = new QWidget;
  auto *layout = new QVBoxLayout(root);
  layout->setContentsMargins(24, 20, 24, 20);
  layout->setSpacing(16);
  auto *title = new QLabel("DroidCast");
  auto font = title->font();
  font.setPointSize(22);
  title->setFont(font);
  layout->addWidget(title);
  auto *description =
      new QLabel("Espelhamento real · interface experimental\nO vídeo abre na "
                 "janela nativa do motor.");
  description->setWordWrap(true);
  layout->addWidget(description);
  auto *deviceGroup = new QGroupBox("Dispositivo");
  auto *deviceLayout = new QVBoxLayout(deviceGroup);
  devices = new QComboBox;
  devices->setObjectName("liveDevices");
  devices->setAccessibleName("Dispositivo Android");
  deviceLayout->addWidget(devices);
  deviceStatus = new QLabel;
  deviceStatus->setWordWrap(true);
  deviceLayout->addWidget(deviceStatus);
  auto *row = new QHBoxLayout;
  startButton = new QPushButton("Espelhar");
  startButton->setObjectName("liveStart");
  refreshButton = new QPushButton("Atualizar dispositivos");
  row->addWidget(startButton);
  row->addWidget(refreshButton);
  row->addStretch();
  deviceLayout->addLayout(row);
  layout->addWidget(deviceGroup);
  usbOffer = new QGroupBox("Android conectado via USB");
  usbOffer->setObjectName("liveUsbOffer");
  auto *offerLayout = new QVBoxLayout(usbOffer);
  usbOfferText = new QLabel;
  usbOfferText->setObjectName("liveUsbOfferText");
  usbOfferText->setTextFormat(Qt::PlainText);
  usbOfferText->setWordWrap(true);
  offerLayout->addWidget(usbOfferText);
  auto *offerRow = new QHBoxLayout;
  usbMirrorButton = new QPushButton("Espelhar");
  usbMirrorButton->setObjectName("liveUsbMirror");
  usbTransferButton = new QPushButton("Transferir arquivo…");
  usbTransferButton->setObjectName("liveUsbTransfer");
  auto *dismiss = new QPushButton("Agora não");
  dismiss->setObjectName("liveUsbDismiss");
  for (auto *button : {usbMirrorButton, usbTransferButton, dismiss})
    offerRow->addWidget(button);
  offerLayout->addLayout(offerRow);
  layout->addWidget(usbOffer);
  usbOffer->hide();
  usbMirrorAction = new QAction("Espelhar este dispositivo", this);
  usbMirrorAction->setObjectName("liveUsbMirrorAction");
  usbTransferAction = new QAction("Transferir arquivo…", this);
  usbDismissAction = new QAction("Agora não", this);
  connect(dismiss, &QPushButton::clicked, usbDismissAction, &QAction::trigger);
  connect(usbDismissAction, &QAction::triggered, this,
          &LiveWindow::dismissUsbOffer);
  connect(usbMirrorButton, &QPushButton::clicked, usbMirrorAction,
          &QAction::trigger);
  connect(usbMirrorAction, &QAction::triggered, this, [this] {
    if (!selectUsbOfferTarget())
      return;
    dismissUsbOffer();
    startSession();
  });
  connect(usbTransferButton, &QPushButton::clicked, usbTransferAction,
          &QAction::trigger);
  connect(usbTransferAction, &QAction::triggered, this, [this] {
    if (!selectUsbOfferTarget())
      return;
    dismissUsbOffer();
    transfer();
  });
  sessionStatus = new QLabel;
  sessionStatus->setObjectName("liveState");
  sessionStatus->setWordWrap(true);
  sessionStatus->setTextFormat(Qt::PlainText);
  layout->addWidget(sessionStatus);
  auto *sessionRow = new QHBoxLayout;
  fitButton = new QPushButton("Ajustar janela");
  fitButton->setObjectName("liveFit");
  stopButton = new QPushButton("Encerrar sessão");
  stopButton->setObjectName("liveStop");
  sessionRow->addWidget(fitButton);
  sessionRow->addWidget(stopButton);
  sessionRow->addStretch();
  layout->addLayout(sessionRow);
  layout->addStretch();
  feedback = new QLabel("Conecte e autorize o Android para começar.");
  feedback->setObjectName("liveFeedback");
  feedback->setTextFormat(Qt::PlainText);
  feedback->setWordWrap(true);
  layout->addWidget(feedback);
  setCentralWidget(root);

  auto *deviceMenu = menuBar()->addMenu("Dispositivo");
  connect(deviceMenu->addAction("Atualizar dispositivos"), &QAction::triggered,
          this, [this] {
            if (!closing)
              engine->refresh();
          });
  transferAction = deviceMenu->addAction("Transferir arquivo…");
  cancelTransferAction = deviceMenu->addAction("Cancelar transferência");
  connect(transferAction, &QAction::triggered, this, &LiveWindow::transfer);
  connect(cancelTransferAction, &QAction::triggered, engine,
          &Engine::cancelDeviceTask);
  auto *sessionMenu = menuBar()->addMenu("Sessão");
  startAction = sessionMenu->addAction("Espelhar");
  startAction->setObjectName("liveStartAction");
  startAction->setShortcut(QKeySequence("Ctrl+Return"));
  recordAction = sessionMenu->addAction("Iniciar com gravação…");
  captureAction = sessionMenu->addAction("Capturar imagem…");
  stopAction = sessionMenu->addAction("Encerrar sessão");
  stopAction->setObjectName("liveStopAction");
  auto *viewMenu = menuBar()->addMenu("Exibir");
  fitAction = viewMenu->addAction("Ajustar janela");
  fitAction->setObjectName("liveFitAction");
  auto addPresentation = [this, viewMenu](const QString &title,
                                          Engine::WindowAction command) {
    auto *action = viewMenu->addAction(title);
    presentationActions.append(action);
    connect(action, &QAction::triggered, this, [this, action, command] {
      if (action->isEnabled() && !closing)
        engine->windowAction(command);
    });
  };
  addPresentation("Tela cheia", Engine::WindowAction::Fullscreen);
  addPresentation("Tamanho real (1:1)", Engine::WindowAction::PixelPerfect);
  addPresentation("Girar imagem à esquerda", Engine::WindowAction::RotateLeft);
  addPresentation("Girar imagem à direita", Engine::WindowAction::RotateRight);
  viewMenu->addSeparator();
  addPresentation("Pausar imagem", Engine::WindowAction::Pause);
  addPresentation("Retomar imagem", Engine::WindowAction::Resume);
  auto *settingsAction =
      menuBar()->addMenu("Ferramentas")->addAction("Configurações…");
  settingsAction->setMenuRole(QAction::PreferencesRole);
  settingsAction->setShortcut(QKeySequence::Preferences);
  connect(settingsAction, &QAction::triggered, this, &LiveWindow::showSettings);
  connect(startAction, &QAction::triggered, this, [this] { startSession(); });
  connect(startButton, &QPushButton::clicked, this, [this] { startSession(); });
  connect(stopAction, &QAction::triggered, this, &LiveWindow::stopSession);
  connect(stopButton, &QPushButton::clicked, this, &LiveWindow::stopSession);
  connect(fitAction, &QAction::triggered, this, &LiveWindow::fitWindow);
  connect(fitButton, &QPushButton::clicked, this, &LiveWindow::fitWindow);
  connect(recordAction, &QAction::triggered, this, &LiveWindow::recordSession);
  connect(captureAction, &QAction::triggered, this, &LiveWindow::capture);
  connect(refreshButton, &QPushButton::clicked, engine, &Engine::refresh);
  connect(devices, &QComboBox::currentIndexChanged, this, [this] {
    if (!selectedSerial().isEmpty())
      lastSelectedSerial = selectedSerial();
    updateActions();
  });
  auto *help = menuBar()->addMenu("Ajuda");
  logs = new QPlainTextEdit;
  logs->setReadOnly(true);
  logs->setMaximumBlockCount(1000);
  auto *diagnostics = new QDialog(this);
  diagnostics->setWindowTitle("Diagnóstico local");
  diagnostics->resize(720, 440);
  auto *diagnosticsLayout = new QVBoxLayout(diagnostics);
  diagnosticsLayout->addWidget(logs);
  connect(help->addAction("Diagnóstico…"), &QAction::triggered, diagnostics,
          &QDialog::show);
  auto *about = help->addAction("Sobre a interface compacta");
  about->setMenuRole(QAction::AboutRole);
  connect(about, &QAction::triggered, this, [this] {
    QMessageBox::information(
        this, "Interface experimental",
        "Este modo usa o motor real e oferece tray opcional.\nModos "
        "especialistas e configurações completas ainda estão em "
        "migração.\nPausar imagem não pausa Android, áudio nem gravação.\nUse "
        "--demo para a demonstração simulada separada.");
  });
  auto *quit = help->addAction("Sair");
  quit->setMenuRole(QAction::QuitRole);
  quit->setShortcut(QKeySequence::Quit);
  connect(quit, &QAction::triggered, this, &LiveWindow::requestQuit);
  QPixmap icon(44, 44);
  icon.fill(Qt::transparent);
  icon.setDevicePixelRatio(2);
  {
    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(palette().color(QPalette::WindowText), 1.5));
    painter.drawRoundedRect(QRectF(6, 2, 10, 18), 2, 2);
    painter.drawLine(QPointF(9, 16.5), QPointF(13, 16.5));
  }
  QIcon trayIcon(icon);
#ifdef Q_OS_MACOS
  trayIcon.setIsMask(true);
#endif
  tray = new QSystemTrayIcon(trayIcon, this);
  auto *trayMenu = new QMenu(this);
  trayMenu->setObjectName("liveTrayMenu");
  trayState = trayMenu->addAction("DroidCast");
  trayState->setEnabled(false);
  auto *restoreAction = trayMenu->addAction("Abrir DroidCast");
  restoreAction->setObjectName("liveRestoreAction");
  connect(restoreAction, &QAction::triggered, this, &LiveWindow::restoreWindow);
  trayMenu->addSeparator();
  usbOfferMenu = trayMenu->addMenu("Android conectado");
  usbOfferMenu->addAction(usbMirrorAction);
  usbOfferMenu->addAction(usbTransferAction);
  usbOfferMenu->addAction(usbDismissAction);
  trayMenu->addAction(startAction);
  trayMenu->addAction(recordAction);
  trayMenu->addAction(captureAction);
  trayMenu->addAction(transferAction);
  trayMenu->addAction(cancelTransferAction);
  auto *imageMenu = trayMenu->addMenu("Imagem");
  imageMenu->setObjectName("liveTrayImageMenu");
  imageMenu->addAction(fitAction);
  for (auto *action : presentationActions)
    imageMenu->addAction(action);
  trayMenu->addAction(stopAction);
  trayMenu->addSeparator();
  trayMenu->addAction(settingsAction);
  trayMenu->addAction(quit);
  tray->setContextMenu(trayMenu);
  connect(tray, &QSystemTrayIcon::activated, this,
          [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger ||
                reason == QSystemTrayIcon::DoubleClick)
              restoreWindow();
          });
  connect(tray, &QSystemTrayIcon::messageClicked, this,
          &LiveWindow::restoreWindow);
  connect(engine, &Engine::devicesChanged, this, &LiveWindow::updateDevices);
  connect(engine, &Engine::deviceScanCompleted, this,
          &LiveWindow::processDeviceScan);
  connect(engine, &Engine::deviceMonitoringReset, this, [this] {
    for (const auto &serial : knownUsb.keys())
      ++usbGeneration[serial];
    knownUsb.clear();
    pendingUsbOffers.clear();
    usbBaselineReady = false;
    updateUsbOffer();
  });
  connect(engine, &Engine::deviceMonitoringChanged, this,
          &LiveWindow::finishClose);
  connect(engine, &Engine::scanChanged, this, &LiveWindow::updateActions);
  connect(engine, &Engine::sessionChanged, this, [this] {
    updateActions();
    finishClose();
  });
  connect(engine, &Engine::deviceTaskChanged, this, [this] {
    updateActions();
    finishClose();
  });
  connect(engine, &Engine::windowControlsChanged, this,
          &LiveWindow::updateActions);
  connect(engine, &Engine::message, this, [this](const QString &text) {
    feedback->setText(text);
    logs->appendPlainText(text);
  });
  connect(engine, &Engine::windowControlMessage, feedback, &QLabel::setText);
  connect(engine, &Engine::filePushMessage, feedback, &QLabel::setText);
  connect(engine, &Engine::log, logs, &QPlainTextEdit::appendPlainText);
  connect(&trayWatch, &QTimer::timeout, this,
          &LiveWindow::refreshTrayAvailability);
  trayWatch.start(1000);
  if (uiPreferences) {
    trayEnabled = uiPreferences->value("ui/tray", false).toBool();
    backgroundEnabled = uiPreferences->value("ui/background", false).toBool();
    usbOffersEnabled = uiPreferences->value("ui/usbOffers", true).toBool();
  }
  refreshTrayAvailability();
  updateDevices();
  if (discover) {
    QTimer::singleShot(0, this, [this] {
      if (!closing)
        engine->startDeviceMonitoring();
    });
  }
}
LiveWindow::~LiveWindow() {
  tray->hide();
  disconnect(engine, nullptr, this, nullptr);
  disconnect(engine, nullptr, feedback, nullptr);
  disconnect(engine, nullptr, logs, nullptr);
  engine->stopDeviceMonitoring();
}
QString LiveWindow::selectedSerial() const {
  return devices->currentData().toString();
}
void LiveWindow::updateDevices() {
  const auto serial =
      engine->running() ? engine->activeSerial : lastSelectedSerial;
  devices->blockSignals(true);
  devices->clear();
  for (const auto &device : engine->devices)
    devices->addItem(device.model + " · " + device.serial + " · " +
                         device.connection,
                     device.serial);
  const int selected = devices->findData(serial);
  if (selected >= 0)
    devices->setCurrentIndex(selected);
  devices->blockSignals(false);
  if (!selectedSerial().isEmpty())
    lastSelectedSerial = selectedSerial();
  updateActions();
}
void LiveWindow::updateActions() {
  bool ready = false;
  QString state;
  for (const auto &device : engine->devices)
    if (device.serial == selectedSerial()) {
      ready = device.ready();
      state = device.state;
    }
  deviceStatus->setText(
      ready ? "Autorizado · pronto para espelhar"
      : state == "unauthorized"
          ? "Desbloqueie o Android e aceite a autorização de depuração USB."
      : state == "offline" ? "Dispositivo offline. Reconecte e tente novamente."
      : devices->count()   ? "Acesso indisponível. Consulte Diagnóstico."
                           : "Nenhum dispositivo. Conecte o USB e habilite a "
                             "depuração no Android.");
  const bool start =
      ready && !engine->running() && !engine->deviceBusy() && !closing;
  startAction->setEnabled(start);
  startAction->setText(selectedSerial().isEmpty()
                           ? "Espelhar"
                           : "Espelhar · " + selectedSerial());
  transferAction->setText(selectedSerial().isEmpty()
                              ? "Transferir arquivo…"
                              : "Transferir arquivo… · " + selectedSerial());
  startButton->setEnabled(start);
  recordAction->setEnabled(start);
  const bool stop = engine->running() &&
                    engine->sessionState() != Engine::SessionState::Stopping &&
                    !closing;
  stopAction->setEnabled(stop);
  stopButton->setEnabled(stop);
  const bool fit = engine->windowControlsAvailable() && !closing;
  fitAction->setEnabled(fit);
  fitButton->setEnabled(fit);
  for (auto *action : presentationActions)
    action->setEnabled(fit);
  devices->setEnabled(!engine->running() && !closing);
  refreshButton->setEnabled(!engine->scanning() && !closing);
  transferAction->setEnabled(
      ready && !engine->deviceBusy() && !closing &&
      (!engine->running() || selectedSerial() == engine->activeSerial));
  cancelTransferAction->setEnabled(engine->filePushBusy() && !closing);
  captureAction->setEnabled(ready && !engine->deviceBusy() && !closing &&
                            engine->captureAllowed(engine->running()
                                                       ? engine->activeSerial
                                                       : selectedSerial()));
  QString sessionText;
  switch (engine->sessionState()) {
  case Engine::SessionState::Idle:
    sessionText = "Nenhuma sessão ativa";
    break;
  case Engine::SessionState::Starting:
    sessionText = "Iniciando · aguardando vídeo";
    break;
  case Engine::SessionState::Streaming:
    sessionText = "Espelhando";
    break;
  case Engine::SessionState::Stopping:
    sessionText = "Encerrando e finalizando gravação";
    break;
  case Engine::SessionState::Ended:
    sessionText = "Sessão encerrada";
    break;
  case Engine::SessionState::Failed:
    sessionText = "Falha na sessão · consulte Diagnóstico";
    break;
  case Engine::SessionState::Disconnected:
    sessionText = "Dispositivo desconectado";
    break;
  }
  sessionStatus->setText(sessionText + (engine->running()
                                            ? " · " + engine->activeSerial
                                            : QString()));
  trayState->setText(sessionStatus->text());
  tray->setToolTip("DroidCast — " + sessionStatus->text());
  updateUsbOffer();
}
void LiveWindow::startSession(const QString &recording) {
  if (closing || !startAction->isEnabled())
    return;
  engine->start(selectedSerial(), recording);
  updateActions();
}
void LiveWindow::stopSession() { engine->stop(); }
void LiveWindow::fitWindow() {
  if (fitAction->isEnabled())
    engine->windowAction(Engine::WindowAction::Fit);
}
void LiveWindow::recordSession() {
  if (closing || !recordAction->isEnabled())
    return;
  const auto serial = selectedSerial();
  const auto path = QFileDialog::getSaveFileName(
      this, "Iniciar com gravação",
      QDir::homePath() + "/DroidCast-" +
          QUuid::createUuid().toString(QUuid::Id128).left(8) + ".mkv",
      "Matroska (*.mkv)");
  if (closing || path.isEmpty() || selectedSerial() != serial ||
      !startAction->isEnabled())
    return;
  engine->start(serial, path);
}
void LiveWindow::capture() {
  if (closing || !captureAction->isEnabled())
    return;
  const auto serial =
      engine->running() ? engine->activeSerial : selectedSerial();
  const auto path = QFileDialog::getSaveFileName(
      this, "Capturar imagem", QDir::homePath() + "/DroidCast.png",
      "PNG (*.png)");
  if (closing || path.isEmpty() || !captureAction->isEnabled() ||
      (engine->running() ? engine->activeSerial : selectedSerial()) != serial)
    return;
  engine->capture(serial, path);
}
void LiveWindow::transfer() {
  if (closing || !transferAction->isEnabled())
    return;
  const auto serial = selectedSerial();
  const auto generation = usbGeneration.value(serial);
  const auto file =
      QFileDialog::getOpenFileName(this, "Escolher arquivo para " + serial);
  if (closing || file.isEmpty() || selectedSerial() != serial ||
      usbGeneration.value(serial) != generation || !transferAction->isEnabled())
    return;
  bool accepted = false;
  const auto destination = QInputDialog::getText(
      this, "Destino Android — " + serial,
      "Diretório de destino (arquivo com o mesmo nome será substituído):",
      QLineEdit::Normal, "/sdcard/Download/", &accepted);
  if (closing || !accepted || selectedSerial() != serial ||
      usbGeneration.value(serial) != generation || !transferAction->isEnabled())
    return;
  engine->pushFile(serial, file, destination);
}
void LiveWindow::closeEvent(QCloseEvent *event) {
  if (!closing && backgroundEnabled && trayUsable()) {
    event->ignore();
    hide();
    return;
  }
  closing = true;
  engine->stopDeviceMonitoring();
  trayWatch.stop();
  if (engine->running() || engine->deviceBusy() ||
      engine->deviceMonitorRunning()) {
    closing = true;
    event->ignore();
    engine->stop();
    if (engine->filePushBusy())
      engine->cancelDeviceTask();
    feedback->setText("Encerrando operações antes de sair…");
    updateActions();
  } else {
    tray->hide();
    event->accept();
    if (!shutdownReadySent) {
      shutdownReadySent = true;
      emit shutdownReady();
    }
  }
}
bool LiveWindow::trayUsable() const { return trayEnabled && trayAvailable(); }
void LiveWindow::setTrayEnabled(bool enabled) {
  trayEnabled = enabled;
  if (uiPreferences)
    uiPreferences->setValue("ui/tray", enabled);
  refreshTrayAvailability();
}
void LiveWindow::setBackgroundEnabled(bool enabled) {
  backgroundEnabled = enabled;
  if (uiPreferences)
    uiPreferences->setValue("ui/background", enabled);
}
void LiveWindow::refreshTrayAvailability() {
  if (closing)
    return;
  const bool usable = trayUsable();
  if (usable != lastTrayUsable)
    tray->setVisible(usable);
  if (lastTrayUsable && !usable && !isVisible())
    restoreWindow();
  lastTrayUsable = usable;
}
void LiveWindow::restoreWindow() {
  if (closing)
    return;
  showNormal();
  raise();
  activateWindow();
}
void LiveWindow::requestQuit() {
  closing = true;
  close();
}
void LiveWindow::showSettings() {
  if (closing)
    return;
  // Always provide a visible parent before opening a dialog from the tray.
  restoreWindow();
  QDialog dialog(this);
  dialog.setWindowTitle("Configurações");
  auto *layout = new QVBoxLayout(&dialog);
  auto *trayChoice = new QCheckBox("Mostrar ícone no tray / barra de menus");
  trayChoice->setChecked(trayEnabled);
  layout->addWidget(trayChoice);
  auto *backgroundChoice =
      new QCheckBox("Manter em segundo plano ao fechar a janela");
  backgroundChoice->setChecked(backgroundEnabled);
  layout->addWidget(backgroundChoice);
  auto *usbChoice = new QCheckBox("Oferecer ações ao conectar Android via USB");
  usbChoice->setChecked(usbOffersEnabled);
  layout->addWidget(usbChoice);
  auto *note = new QLabel(
      trayAvailable() ? "Fechar pode ocultar a janela; Sair sempre encerra "
                        "operações e o app."
                      : "Tray indisponível neste desktop. Fechar encerrará o "
                        "app; a janela permanece acessível.");
  note->setWordWrap(true);
  layout->addWidget(note);
  auto updateDependency = [&] {
    backgroundChoice->setEnabled(trayChoice->isChecked() && trayAvailable());
  };
  connect(trayChoice, &QCheckBox::toggled, &dialog,
          [&](bool) { updateDependency(); });
  updateDependency();
  auto *planned =
      new QLabel("Iniciar com o sistema ainda não está habilitado.\n"
                 "A oferta acompanha eventos do ADB e requer depuração USB.\n"
                 "Pausar imagem não pausa áudio ou gravação.");
  planned->setWordWrap(true);
  layout->addWidget(planned);
  auto *buttons =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  if (dialog.exec() == QDialog::Accepted && !closing) {
    setBackgroundEnabled(backgroundChoice->isChecked());
    setTrayEnabled(trayChoice->isChecked());
    setUsbOffersEnabled(usbChoice->isChecked());
  }
}
void LiveWindow::finishClose() {
  if (closing && !engine->running() && !engine->deviceBusy() &&
      !engine->deviceMonitorRunning())
    QTimer::singleShot(0, this, [this] { close(); });
}

void LiveWindow::setUsbOffersEnabled(bool enabled) {
  usbOffersEnabled = enabled;
  if (uiPreferences)
    uiPreferences->setValue("ui/usbOffers", enabled);
  if (!enabled)
    pendingUsbOffers.clear();
  updateUsbOffer();
}

void LiveWindow::processDeviceScan(bool success) {
  if (closing || !success)
    return;
  if (engine->preferences.remoteAdb) {
    usbBaselineReady = false;
    knownUsb.clear();
    pendingUsbOffers.clear();
    updateUsbOffer();
    return;
  }
  QHash<QString, int> counts;
  for (const auto &device : engine->devices)
    if (device.connection == "USB")
      ++counts[device.serial];
  QHash<QString, Device> current;
  // Preserve report order for multiple arrivals; never auto-select another
  // target.
  for (const auto &device : engine->devices) {
    if (device.connection != "USB" || device.serial.isEmpty() ||
        counts.value(device.serial) != 1)
      continue;
    current.insert(device.serial, device);
    if (!knownUsb.contains(device.serial)) {
      ++usbGeneration[device.serial];
      if (usbBaselineReady && usbOffersEnabled)
        pendingUsbOffers.append(device.serial);
    }
  }
  knownUsb = current;
  usbBaselineReady = true;
  for (qsizetype i = pendingUsbOffers.size(); i > 0; --i)
    if (!knownUsb.contains(pendingUsbOffers[i - 1]))
      pendingUsbOffers.removeAt(i - 1);
  updateUsbOffer();
}

void LiveWindow::updateUsbOffer() {
  if (closing || !usbOffersEnabled || pendingUsbOffers.isEmpty() ||
      engine->preferences.remoteAdb) {
    usbOffer->hide();
    usbOfferMenu->menuAction()->setVisible(false);
    usbMirrorAction->setEnabled(false);
    usbTransferAction->setEnabled(false);
    usbMirrorButton->setEnabled(false);
    usbTransferButton->setEnabled(false);
    tray->setToolTip("DroidCast — " + sessionStatus->text());
    announcedUsb.clear();
    return;
  }
  const auto serial = pendingUsbOffers.first();
  const auto device = knownUsb.value(serial);
  bool ready = false;
  for (const auto &entry : engine->devices)
    if (entry.serial == serial && entry.connection == "USB")
      ready = entry.ready();
  const bool busy = engine->running() || engine->deviceBusy();
  const bool enabled = ready && !busy && !closing;
  usbMirrorButton->setEnabled(enabled);
  usbTransferButton->setEnabled(enabled);
  usbMirrorAction->setEnabled(enabled);
  usbTransferAction->setEnabled(enabled);
  const auto explanation =
      busy    ? "Outra operação está em andamento. Nenhuma sessão será "
                "interrompida."
      : ready ? "O que deseja fazer? Nenhuma ação começa automaticamente."
              : "Desbloqueie o Android e autorize a depuração USB. Se "
                "necessário, atualize os dispositivos.";
  usbOfferText->setText(device.model + " · " + serial + "\n" + explanation +
                        (pendingUsbOffers.size() > 1
                             ? QString("\nMais %1 dispositivo(s) aguardando.")
                                   .arg(pendingUsbOffers.size() - 1)
                             : QString()));
  usbOffer->show();
  usbOfferMenu->setTitle("USB: " + device.model + " · " + serial);
  usbOfferMenu->menuAction()->setVisible(true);
  if (announcedUsb != serial) {
    announcedUsb = serial;
    if (trayUsable() && (!isVisible() || isMinimized()) &&
        QSystemTrayIcon::supportsMessages())
      tray->showMessage(
          "Android conectado via USB",
          "Abra DroidCast para escolher Espelhar ou Transferir arquivo…");
  }
  if (trayUsable())
    tray->setToolTip("Android USB aguardando: " + device.model + " · " +
                     serial);
}

void LiveWindow::dismissUsbOffer() {
  if (!pendingUsbOffers.isEmpty())
    pendingUsbOffers.removeFirst();
  announcedUsb.clear();
  updateUsbOffer();
}

bool LiveWindow::selectUsbOfferTarget() {
  if (closing || !usbOffersEnabled || engine->preferences.remoteAdb ||
      pendingUsbOffers.isEmpty() || engine->running() || engine->deviceBusy())
    return false;
  const auto serial = pendingUsbOffers.first();
  int count = 0;
  for (const auto &device : engine->devices)
    if (device.serial == serial && device.connection == "USB" && device.ready())
      ++count;
  if (count != 1 || !knownUsb.contains(serial))
    return false;
  const int index = devices->findData(serial);
  if (index < 0)
    return false;
  devices->setCurrentIndex(index);
  return true;
}
