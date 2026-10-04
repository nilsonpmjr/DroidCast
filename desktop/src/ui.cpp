#include "ui.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPalette>
#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_MACOS)
#include <sys/sysctl.h>
#endif

namespace {
bool laptopHost() {
#ifdef Q_OS_WIN
  SYSTEM_POWER_STATUS status;
  return GetSystemPowerStatus(&status) && status.BatteryFlag != 128;
#elif defined(Q_OS_MACOS)
  size_t size = 0;
  if (sysctlbyname("hw.model", nullptr, &size, nullptr, 0) || !size)
    return false;
  QByteArray model(static_cast<qsizetype>(size), '\0');
  return !sysctlbyname("hw.model", model.data(), &size, nullptr, 0) &&
         model.startsWith("MacBook");
#elif defined(Q_OS_LINUX)
  const QDir supplies("/sys/class/power_supply");
  for (const auto &entry :
       supplies.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    QFile type(supplies.filePath(entry + "/type"));
    if (type.open(QIODevice::ReadOnly) && type.readAll().trimmed() == "Battery")
      return true;
  }
#endif
  return false;
}

QPixmap croppedArtwork(const QString &resource) {
  QImage image(resource);
  if (image.isNull())
    return {};
  image = image.convertToFormat(QImage::Format_RGBA8888);
  int left = image.width(), top = image.height(), right = -1, bottom = -1;
  for (int y = 0; y < image.height(); ++y) {
    const auto *line = image.constScanLine(y);
    for (int x = 0; x < image.width(); ++x) {
      if (line[x * 4 + 3] < 8)
        continue;
      left = qMin(left, x);
      right = qMax(right, x);
      top = qMin(top, y);
      bottom = qMax(bottom, y);
    }
  }
  return right >= left ? QPixmap::fromImage(image.copy(
                             left, top, right - left + 1, bottom - top + 1))
                       : QPixmap{};
}
} // namespace

QSize PreferenceSwitch::sizeHint() const {
  return QSize(fontMetrics().horizontalAdvance(text()) + 58,
               qMax(34, fontMetrics().height() + 12));
}
void PreferenceSwitch::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  const int y = (height() - 20) / 2;
  p.setPen(Qt::NoPen);
  p.setBrush(isChecked() ? QColor("#245bc2") : QColor("#536277"));
  p.drawRoundedRect(QRectF(3, y, 34, 20), 10, 10);
  p.setBrush(isEnabled() ? QColor("#edf1f7") : QColor("#8894a4"));
  p.drawEllipse(QRectF(isChecked() ? 20 : 6, y + 3, 14, 14));
  p.setPen(isEnabled() ? QColor("#edf1f7") : QColor("#8894a4"));
  p.drawText(QRect(48, 0, width() - 50, height()),
             Qt::AlignLeft | Qt::AlignVCenter, text());
  if (hasFocus()) {
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor("#91bbff"), 2));
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5, 5);
  }
}

QIcon appIcon(const QString &name, const QColor &color) {
  QPixmap pix(48, 48);
  pix.fill(Qt::transparent);
  QPainter p(&pix);
  p.setRenderHint(QPainter::Antialiasing);
  p.scale(2, 2);
  p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  p.setBrush(Qt::NoBrush);
  if (name == "phone") {
    p.drawRoundedRect(QRectF(6, 2, 12, 20), 2, 2);
    p.drawLine(10, 5, 14, 5);
    p.drawPoint(12, 19);
  } else if (name == "mirror" || name == "brand") {
    p.drawRoundedRect(QRectF(2, 3, 16, 13), 2, 2);
    p.drawLine(10, 16, 10, 21);
    p.drawLine(5, 21, 15, 21);
    p.setBrush(QColor("#20252d"));
    p.drawRoundedRect(QRectF(15, 9, 7, 13), 1.5, 1.5);
  } else if (name == "wifi") {
    p.drawArc(QRectF(2, 3, 20, 20), 35 * 16, 110 * 16);
    p.drawArc(QRectF(6, 8, 12, 12), 35 * 16, 110 * 16);
    p.drawPoint(12, 19);
  } else if (name == "capture") {
    p.drawRoundedRect(QRectF(2, 6, 20, 15), 2, 2);
    p.drawEllipse(QPointF(12, 13), 4, 4);
    p.drawLine(7, 6, 9, 3);
    p.drawLine(9, 3, 15, 3);
    p.drawLine(15, 3, 17, 6);
  } else if (name == "input") {
    p.drawRoundedRect(QRectF(2, 5, 20, 14), 2, 2);
    for (int x : {6, 10, 14, 18})
      for (int y : {9, 12})
        p.drawPoint(x, y);
    p.drawLine(7, 16, 17, 16);
  } else if (name == "settings") {
    for (int y : {6, 12, 18})
      p.drawLine(3, y, 21, y);
    p.setBrush(QColor("#20252d"));
    p.drawEllipse(QPointF(8, 6), 2, 2);
    p.drawEllipse(QPointF(16, 12), 2, 2);
    p.drawEllipse(QPointF(10, 18), 2, 2);
  } else if (name == "folder") {
    p.drawPolygon(
        QPolygonF{{2, 7}, {2, 4}, {9, 4}, {12, 7}, {22, 7}, {22, 20}, {2, 20}});
  } else if (name == "play") {
    p.drawPolygon(QPolygonF{{7, 4}, {20, 12}, {7, 20}});
  } else if (name == "stop") {
    p.drawRoundedRect(QRectF(5, 5, 14, 14), 2, 2);
  } else if (name == "refresh") {
    p.drawArc(QRectF(4, 4, 16, 16), 35 * 16, 290 * 16);
    p.drawLine(20, 3, 20, 9);
    p.drawLine(14, 9, 20, 9);
  } else if (name == "video") {
    p.drawRoundedRect(QRectF(2, 5, 14, 14), 2, 2);
    p.drawPolyline(QPolygonF{{16, 9}, {22, 6}, {22, 18}, {16, 15}});
  } else {
    p.drawRoundedRect(QRectF(2, 4, 20, 16), 2, 2);
    p.drawPolyline(QPolygonF{{6, 9}, {9, 12}, {6, 15}});
    p.drawLine(12, 15, 17, 15);
  }
  return QIcon(pix);
}
ConnectionArt::ConnectionArt(QWidget *parent) : QWidget(parent) {
  const bool laptop = laptopHost();
  artwork = croppedArtwork(laptop ? ":/branding/laptop.png"
                                  : ":/branding/desktop.png");
  setMinimumSize(240, 160);
  setMaximumHeight(220);
  setAccessibleName(laptop ? "DroidCast laptop artwork"
                           : "DroidCast desktop artwork");
}
void ConnectionArt::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::SmoothPixmapTransform);
  if (artwork.isNull())
    return;
  const auto target =
      artwork.size().scaled(size() - QSize(16, 16), Qt::KeepAspectRatio);
  p.drawPixmap(QRect(QPoint((width() - target.width()) / 2,
                            (height() - target.height()) / 2),
                     target),
               artwork);
}
void applyTheme(QApplication &app) {
  app.setStyle("Fusion");
  QPalette palette;
  palette.setColor(QPalette::Window, QColor("#20252d"));
  palette.setColor(QPalette::WindowText, QColor("#edf1f7"));
  palette.setColor(QPalette::Base, QColor("#171d25"));
  palette.setColor(QPalette::AlternateBase, QColor("#293341"));
  palette.setColor(QPalette::Text, QColor("#edf1f7"));
  palette.setColor(QPalette::Button, QColor("#303c4b"));
  palette.setColor(QPalette::ButtonText, QColor("#edf1f7"));
  palette.setColor(QPalette::Highlight, QColor("#245bc2"));
  palette.setColor(QPalette::HighlightedText, Qt::white);
  palette.setColor(QPalette::PlaceholderText, QColor("#b4c0d0"));
  app.setPalette(palette);
  app.setStyleSheet(R"(
        QWidget { color: #edf1f7; font-size: 13px; }
        QMainWindow, QWidget#workspace, QScrollArea, QStackedWidget { background: #20252d; }
        QWidget#sidebar { background: #171d25; border-right: 1px solid #354152; }
        QLabel { background: transparent; }
        QLabel[role="title"] { font-size: 23px; font-weight: 600; }
        QLabel[role="brand"] { font-size: 20px; font-weight: 600; }
        QLabel[role="section"] { font-size: 15px; font-weight: 600; }
        QLabel[role="muted"] { color: #b4c0d0; }
        QLabel[role="badge"] { color: #bcd6ff; background: #293e5c; border-radius: 5px; padding: 5px 9px; }
        QFrame[role="card"] { background: #252e3a; border: 1px solid #465367; border-radius: 10px; }
        QFrame[role="well"] { background: #171d25; border: 1px solid #465367; border-radius: 8px; }
        QFrame[role="card"] QLabel, QFrame[role="well"] QLabel { border: none; }
        QGroupBox { background: #252e3a; border: 1px solid #465367; border-radius: 10px; margin-top: 14px; padding: 20px 16px 16px; }
        QGroupBox::title { subcontrol-origin: margin; left: 18px; padding: 0 6px; color: #edf1f7; font-weight: 600; }
        QPushButton { background: #303c4b; border: 1px solid #64748b; border-radius: 6px; padding: 7px 13px; }
        QPushButton:hover { background: #3d4d61; border-color: #91bbff; }
        QPushButton:pressed { background: #182c46; }
        QPushButton[primary="true"] { background: #245bc2; color: white; border-color: #79a5f4; }
        QPushButton[primary="true"]:hover { background: #306bd7; }
        QPushButton[danger="true"] { color: #ffb6b6; border-color: #9b626b; background: #44303a; }
        QPushButton:disabled { background: #29303a; color: #8894a4; border-color: #414b59; }
        QPushButton:focus, QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QListWidget:focus, QCheckBox:focus { border: 2px solid #91bbff; }
        QLineEdit, QSpinBox, QComboBox { background: #171d25; border: 1px solid #68788e; border-radius: 5px; padding: 7px; min-height: 18px; }
        QComboBox QAbstractItemView { background: #171d25; color: #edf1f7; selection-background-color: #245bc2; }
        QCheckBox { spacing: 10px; padding: 5px 0; border: 2px solid transparent; }
        QCheckBox::indicator { width: 18px; height: 18px; }
        QListWidget { background: #171d25; border: 1px solid #465367; border-radius: 8px; padding: 5px; outline: none; }
        QListWidget::item { padding: 12px; border-radius: 5px; margin-bottom: 5px; }
        QListWidget::item:selected { background: #2a466d; color: white; }
        QListWidget#devices { background: transparent; border: none; padding: 0; }
        QListWidget#devices::item { padding: 0; margin-bottom: 10px; }
        QListWidget#navigation { background: transparent; border: none; padding: 0; }
        QListWidget#navigation::item { padding: 12px 10px; margin: 3px 0; }
        QPlainTextEdit { background: #171d25; border: 1px solid #546173; border-radius: 8px; padding: 12px; }
        QLabel#notice { background: #182333; color: #d8e6fb; padding: 10px 24px; border-top: 1px solid #465367; }
        QWidget#topbar { background: #20252d; border-bottom: 1px solid #354152; }
        QScrollArea { border: none; }
        QToolTip { background: #171d25; color: #edf1f7; border: 1px solid #91bbff; padding: 5px; }
    )");
}
