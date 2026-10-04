#pragma once
#include <QCheckBox>
#include <QIcon>
#include <QPixmap>
#include <QWidget>
class QApplication;
QIcon appIcon(const QString &name, const QColor &color = QColor("#b4c0d0"));
void applyTheme(QApplication &app);
class PreferenceSwitch : public QCheckBox {
public:
  explicit PreferenceSwitch(const QString &text) : QCheckBox(text) {}
  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *) override;
  bool hitButton(const QPoint &point) const override {
    return rect().contains(point);
  }
};
class ConnectionArt : public QWidget {
public:
  explicit ConnectionArt(QWidget *parent = nullptr);

protected:
  void paintEvent(QPaintEvent *) override;

private:
  QPixmap artwork;
};
