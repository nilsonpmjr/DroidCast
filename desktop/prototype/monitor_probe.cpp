#include "engine.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTextStream>

// Manual, read-only probe for a real staged ADB. Never part of automatic CTest.
int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  app.setApplicationName("DroidCast monitor probe");
  Engine engine;
  engine.preferences = bundledPreferences();
  QElapsedTimer elapsed;
  elapsed.start();
  QTextStream output(stdout);
  int snapshots = 0;
  bool stopping = false, healthyAtStop = false;
  QObject::connect(&engine, &Engine::deviceScanCompleted, &app, [&](bool success) {
    if (success) {
      ++snapshots;
      output << "snapshot " << snapshots << ": " << engine.devices.size()
             << " device(s), " << elapsed.elapsed() << " ms\n";
      output.flush();
    }
  });
  QObject::connect(&engine, &Engine::message, &app, [&](const QString &message) {
    output << message << '\n'; output.flush();
  });
  QObject::connect(&engine, &Engine::deviceMonitoringChanged, &app, [&] {
    if (stopping && !engine.deviceMonitorRunning()) app.exit(healthyAtStop ? 0 : 1);
  });
  engine.startDeviceMonitoring();
  QTimer::singleShot(5000, &app, [&] {
    stopping = true;
    healthyAtStop = snapshots > 0 && engine.deviceMonitorRunning();
    output << "5-second probe: " << snapshots << " snapshot(s); stopping owned client\n";
    output.flush();
    engine.stopDeviceMonitoring();
    if (!engine.deviceMonitorRunning()) app.exit(healthyAtStop ? 0 : 1);
  });
  return app.exec();
}
