#include "livewindow.h"
#include "window.h"
#include <QApplication>
#include <QTimer>
#include <memory>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  app.setApplicationName("DroidCast Prototype");
  app.setOrganizationName("DroidCast Prototype");
  const auto args = app.arguments();
  const bool demo = args.contains("--demo") || args.contains("--connected") ||
                    args.contains("--unauthorized") ||
                    args.contains("--session");
  std::unique_ptr<QMainWindow> window;
  if (demo) {
    auto *simulation = new PrototypeWindow(args.contains("--no-tray"));
    if (args.contains("--connected"))
      simulation->simulateConnection();
    if (args.contains("--unauthorized"))
      simulation->simulateConnection(false);
    if (args.contains("--session")) {
      simulation->simulateConnection();
      simulation->startDemo();
    }
    window.reset(simulation);
  } else {
    app.setQuitOnLastWindowClosed(false);
    auto *live = new LiveWindow;
    QObject::connect(live, &LiveWindow::shutdownReady, &app,
                     &QApplication::quit);
    window.reset(live);
  }
  window->show();
  const int screenshot = args.indexOf("--screenshot");
  if (screenshot >= 0 && screenshot + 1 < args.size())
    QTimer::singleShot(500, window.get(), [&] {
      app.exit(window->grab().save(args[screenshot + 1]) ? 0 : 1);
    });
  return app.exec();
}
