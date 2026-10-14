#include "window.h"
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QtTest>

class PrototypeTests : public QObject {
  Q_OBJECT
private slots:
  void changingDeviceInvalidatesPendingOffer() {
    PrototypeWindow window(true);
    window.simulateConnection();
    window.findChild<QComboBox *>("deviceChoice")->setCurrentIndex(1);
    QVERIFY(!window.offerVisible());
    QVERIFY(!window.findChild<QAction *>("startAction")->isEnabled());
    window.startDemo();
    QVERIFY(!window.sessionActive());
    window.simulateConnection();
    QVERIFY(window.offerVisible());
  }
  void connectionNeverStartsAutomatically() {
    PrototypeWindow window(true);
    window.simulateConnection();
    QVERIFY(window.offerVisible());
    QVERIFY(!window.sessionActive());
    window.findChild<QPushButton *>("dismissOffer")->click();
    window.simulateConnection();
    QVERIFY(!window.offerVisible());
    window.simulateDisconnect();
    window.simulateConnection();
    QVERIFY(window.offerVisible());
  }
  void authorizationAndUnplugRevalidateActions() {
    PrototypeWindow window(true);
    window.simulateConnection(false);
    auto *mirror = window.findChild<QPushButton *>("offerMirror");
    QVERIFY(!mirror->isEnabled());
    window.startDemo();
    QVERIFY(!window.sessionActive());
    window.simulateAuthorization();
    QVERIFY(mirror->isEnabled());
    window.simulateDisconnect();
    mirror->click();
    QVERIFY(!window.sessionActive());
    QVERIFY(!window.findChild<QAction *>("transferAction")->isEnabled());
  }
  void suppressOfferAndKeepModePinned() {
    PrototypeWindow window(true);
    window.simulateConnection();
    window.findChild<QCheckBox *>("suppressOffers")->setChecked(true);
    window.findChild<QPushButton *>("dismissOffer")->click();
    window.simulateDisconnect();
    window.simulateConnection();
    QVERIFY(!window.offerVisible());
    window.findChild<QComboBox *>("sessionMode")->setCurrentIndex(2);
    window.findChild<QAction *>("startAction")->trigger();
    QVERIFY(window.sessionActive());
    QVERIFY(!window.findChild<QComboBox *>("sessionMode")->isEnabled());
    window.findChild<QPushButton *>("stopSession")->click();
    QVERIFY(!window.sessionActive());
  }
  void noTrayKeepsWindowWorkflowAvailable() {
    PrototypeWindow window(true);
    window.setTrayEnabled(true);
    QVERIFY(!window.trayUsable());
    window.simulateConnection();
    window.findChild<QPushButton *>("offerMirror")->click();
    QVERIFY(window.sessionActive());
    window.findChild<QAction *>("stopAction")->trigger();
    QVERIFY(!window.sessionActive());
  }
};
QTEST_MAIN(PrototypeTests)
#include "test_window.moc"
