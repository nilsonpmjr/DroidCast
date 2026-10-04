#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QTextStream>
#include <QTimer>
#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <poll.h>
#include <unistd.h>
#endif

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  const auto args = app.arguments();
  QTextStream out(stdout);
  const auto mode = qEnvironmentVariable("HUB_TEST_MODE");
  if (mode == "hang")
    return app.exec();
  if (args.contains("--list-displays")) {
    if (mode == "inspect-hang")
      return app.exec();
    if (mode == "inspect-fail") {
      out << "Device unavailable";
      return 1;
    }
    if (mode == "inspect-overflow") {
      out << QString(1100000, 'x');
      return 0;
    }
    out << args.join('|')
        << "\nList of displays:\n --display-id=0 (1080x2400)\n"
        << " --display-id=2 (1920x1080)\n"
        << "List of video encoders:\n --video-codec=h264 "
           "--video-encoder=c2.android.avc.encoder (hw)\n"
        << " --video-codec=h264 --video-encoder=OMX.vendor.avc (hybrid)\n"
        << " --video-codec=h265 --video-encoder=c2.android.hevc.encoder (hw)\n"
        << "List of cameras:\n --camera-id=0 (back, 1920x1080, "
           "fps={30, 60}, zoom-range=[1, 8])\n"
        << "   - 1920x1080\n   - 1280x720\n"
        << "   High speed capture (--camera-high-speed):\n"
        << "     - 1280x720 (fps={120, 240})\n"
        << "List of apps:\n"
        << " * Settings                      com.android.settings\n"
        << " - Calculator                    com.example.calculator\n"
        << " - A deliberately long application name\n"
        << "                               com.example.longname\n";
    return 0;
  }
  if (args.contains("keyevent")) {
    out << args.join('|');
    return mode == "command-fail" ? 1 : 0;
  }
  if (args.contains("screencap")) {
    if (mode == "invalid-png") {
      out << "not a PNG";
      return 0;
    }
    QFile output;
    if (!output.open(stdout, QIODevice::WriteOnly))
      return 1;
    QImage image(8, 8, QImage::Format_RGB32);
    image.fill(Qt::blue);
    return image.save(&output, "PNG") ? 0 : 1;
  }
  if (args.contains("install")) {
    out << (mode == "install-fail" ? "Failure [INSTALL_FAILED]" : "Success");
    return 0;
  }
  if (args.contains("devices")) {
    if (mode == "scan-fail")
      return 1;
    out << "List of devices attached\nPHONE123 device usb:1-1 model:Pixel_Test "
           "transport_id:1\nLOCKED unauthorized usb:1-2\n";
    return 0;
  }
  if (args.contains("connect") || args.contains("pair")) {
    if (mode == "wireless-fail") {
      out << "failed to connect: connection refused\n";
      return 0;
    }
    if (args.contains("pair")) {
      QFile input;
      if (!input.open(stdin, QIODevice::ReadOnly))
        return 1;
      const auto code = input.readLine().trimmed();
      if (code != "123456")
        return 1;
      out << "Successfully paired to " << args.last() << '\n';
    } else
      out << "connected to " << args.last() << '\n';
    return 0;
  }
  if (mode == "mirror-fail")
    return 1;
  out << "Test mirror process\n";
  out.flush();
  const auto prefix =
      "DROIDCAST/1 " + qEnvironmentVariable("DROIDCAST_SESSION_TOKEN") + " ";
  auto report = [&](const QString &event) {
    out << prefix << event << '\n';
    out.flush();
  };
  report("bridge-ready");
  // Log text and events from a different session cannot establish readiness.
  out << "first-frame\nDROIDCAST/1 00000000000000000000000000000000 "
         "first-frame\n";
  out.flush();
  if (mode != "no-frame")
    QTimer::singleShot(100, &app, [&] {
      report("first-frame");
      if (mode != "old-bridge") {
        report("window-controls-ready");
        if (!args.contains("--video-source=camera") &&
            !args.contains("--no-control") && mode != "old-android-bridge")
          report("android-controls-ready");
        else if (args.contains("--video-source=camera") &&
                 !args.contains("--no-control") && mode != "old-camera-bridge")
          report("camera-controls-ready");
      }
    });
  if (mode == "disconnect")
    QTimer::singleShot(250, &app, [&] {
      report("disconnected");
      app.exit(2);
    });
  QTimer commands;
  QObject::connect(&commands, &QTimer::timeout, &app, [&] {
    if (mode == "ignore-quit")
      return;
    char command;
#ifdef Q_OS_WIN
    DWORD available, received;
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    if (!PeekNamedPipe(input, nullptr, 0, nullptr, &available, nullptr) ||
        !available)
      return;
    if (!ReadFile(input, &command, 1, &received, nullptr) || received != 1)
      return;
#else
    struct pollfd input = {STDIN_FILENO, POLLIN, 0};
    if (poll(&input, 1, 0) <= 0 || read(STDIN_FILENO, &command, 1) != 1) return;
#endif
    if (QByteArray("FWZLRPU").contains(command)) {
      if (mode != "window-timeout")
        report(QString("window-result:%1:%2")
                   .arg(QChar(command), mode == "window-unavailable"
                                            ? "unavailable"
                                            : "handled"));
      return;
    }
    if (QByteArray("Tt+-").contains(command)) {
      if (mode != "camera-timeout")
        report(QString("camera-result:%1:%2")
                   .arg(QChar(command), mode == "camera-unavailable"
                                            ? "unavailable"
                                            : "handled"));
      return;
    }
    if (QByteArray("01NSCDV").contains(command)) {
      if (mode != "android-timeout")
        report(QString("android-result:%1:%2")
                   .arg(QChar(command), mode == "android-unavailable"
                                            ? "unavailable"
                                            : "handled"));
      return;
    }
    if (command != 'Q')
      return;
    for (const auto &arg : args)
      if (arg.startsWith("--record="))
        report(mode == "recording-error" ? "recording-error"
                                         : "recording-finalized");
    report("ended");
    app.quit();
  });
  commands.start(10);
  QTimer::singleShot(30000, &app, &QCoreApplication::quit);
  return app.exec();
}
