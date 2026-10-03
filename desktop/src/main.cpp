#include "ui.h"
#include "window.h"
#include <QApplication>
#include <QTimer>
int main(int argc, char **argv) {
  QApplication app(argc, argv);
  app.setOrganizationName("DroidCast");
  app.setApplicationName("DroidCast Desktop");
  app.setApplicationVersion("0.2.0");
  applyTheme(app);
  Window window;
  window.show();
  const auto args = app.arguments();
  const int page = args.indexOf("--page");
  if (page >= 0 && page + 1 < args.size())
    window.showPage(args[page + 1].toInt());
  const int screenshot = args.indexOf("--screenshot");
  if (screenshot >= 0 && screenshot + 1 < args.size())
    QTimer::singleShot(1000, &window, [&] {
      app.exit(window.grab().save(args[screenshot + 1]) ? 0 : 1);
    });
  return app.exec();
}
