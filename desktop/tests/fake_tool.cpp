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

static bool readAvailableInput(char *byte) {
#ifdef Q_OS_WIN
  DWORD available, received;
  HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
  return PeekNamedPipe(input, nullptr, 0, nullptr, &available, nullptr) &&
         available && ReadFile(input, byte, 1, &received, nullptr) &&
         received == 1;
#else
  struct pollfd input = {STDIN_FILENO, POLLIN, 0};
  return poll(&input, 1, 0) > 0 && read(STDIN_FILENO, byte, 1) == 1;
#endif
}

static bool readExactInput(char *data, qsizetype size) {
  qsizetype receivedTotal = 0;
  while (receivedTotal < size) {
#ifdef Q_OS_WIN
    DWORD received;
    if (!ReadFile(GetStdHandle(STD_INPUT_HANDLE), data + receivedTotal,
                  static_cast<DWORD>(size - receivedTotal), &received,
                  nullptr) ||
        !received)
      return false;
#else
    const auto received =
        read(STDIN_FILENO, data + receivedTotal, size - receivedTotal);
    if (received <= 0)
      return false;
#endif
    receivedTotal += received;
  }
  return true;
}

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  const auto args = app.arguments();
  QTextStream out(stdout);
  const auto mode = qEnvironmentVariable("HUB_TEST_MODE");
  const auto environmentLog = qEnvironmentVariable("HUB_ENV_LOG");
  if (!environmentLog.isEmpty()) {
    QFile log(environmentLog);
    if (log.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
      QTextStream stream(&log);
      stream << args.mid(1).join('|') << '\t'
             << qEnvironmentVariable("ADB_SERVER_SOCKET") << '\n';
    }
  }
  if (mode == "hang")
    return app.exec();
  if (args.contains("track-devices")) {
    auto frame = [](const QByteArray &payload) {
      return QByteArray::number(payload.size(), 16).rightJustified(4, '0') + payload;
    };
    auto write = [&](const QByteArray &bytes) {
      out << QString::fromLatin1(bytes); out.flush();
    };
    const QByteArray phone = "PHONE123\tdevice usb:1-1 model:Pixel_Test transport_id:1\n";
    if (mode == "monitor-malformed") { write("ZZZZ"); return app.exec(); }
    if (mode == "monitor-truncated") { write("001"); return 1; }
    if (mode == "monitor-fail") return 1;
    if (mode == "monitor-fragments") {
      const auto packet = frame(phone);
      write(packet.left(2));
      QTimer::singleShot(20, &app, [&, packet] { write(packet.mid(2, 7)); });
      QTimer::singleShot(40, &app, [&, packet] { write(packet.mid(9) + frame({}) + frame(phone)); });
    } else if (mode == "monitor-events") {
      write(frame({}));
      QTimer::singleShot(100, &app, [&] { write(frame("NEW\tunauthorized usb:1-2\n")); });
      QTimer::singleShot(200, &app, [&] { write(frame("NEW\tdevice usb:1-2 model:Phone\n")); });
    } else {
      write(frame(phone + "LOCKED\tunauthorized usb:1-2\n"));
      if (mode == "monitor-disconnect") QTimer::singleShot(100, &app, [&] { app.exit(1); });
    }
    return app.exec();
  }
  if (args.contains("--list-usb-devices")) {
    if (mode == "otg-inspect-fail")
      return 1;
    out << "DROIDCAST_USB/1 USB-OTG-123\t18d1:4ee7\tGoogle\tPixel\n"
           "DROIDCAST_USB/1 USB-OTG-456\t1234:abcd\tExample\tPhone\n";
    return 0;
  }
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
        << "List of audio encoders:\n --audio-codec=opus "
           "--audio-encoder=c2.android.opus.encoder (sw)\n"
        << " --audio-codec=aac --audio-encoder=c2.android.aac.encoder (sw)\n"
        << " --audio-codec=flac --audio-encoder=c2.android.flac.encoder "
           "(sw)\n"
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
  if (args.contains("push")) {
    if (mode == "push-hang")
      return app.exec();
    out << args.join('|') << '\n';
    out << (mode == "push-fail" ? "adb: error: failed to copy"
                                : "1 file pushed. 0 skipped.");
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
  const bool otg = args.contains("--otg");
  if (mode != "old-audio-lifecycle")
    report("audio-lifecycle-ready");
  // Log text and events from a different session cannot establish readiness.
  out << "first-frame\nDROIDCAST/1 00000000000000000000000000000000 "
         "first-frame\n";
  out.flush();
  const bool audioOnly = args.contains("--no-video");
  if (mode != "no-frame" && mode != "old-audio-lifecycle" &&
      mode != "old-otg-lifecycle")
    QTimer::singleShot(100, &app, [&] {
      report(otg ? "otg-ready"
                 : audioOnly ? "first-audio-packet" : "first-frame");
      if (otg || audioOnly)
        return;
      if (mode != "old-bridge") {
        report("window-controls-ready");
        if (mode != "old-fps-bridge")
          report("fps-controls-ready");
        if (!args.contains("--video-source=camera") &&
            !args.contains("--no-control")) {
          if (mode != "old-android-bridge")
            report("android-controls-ready");
          if (mode != "old-clipboard-bridge")
            report("clipboard-controls-ready");
          if (mode != "old-app-bridge")
            report("app-controls-ready");
        } else if (args.contains("--video-source=camera") &&
                   !args.contains("--no-control") &&
                   mode != "old-camera-bridge")
          report("camera-controls-ready");
      }
    });
  if (mode == "disconnect" || mode == "fps-disconnect")
    QTimer::singleShot(mode == "fps-disconnect" ? 500 : 250, &app, [&] {
      report("disconnected");
      app.exit(2);
    });
  QTimer commands;
  QObject::connect(&commands, &QTimer::timeout, &app, [&] {
    if (mode == "ignore-quit")
      return;
    char command;
    if (!readAvailableInput(&command))
      return;
    if (command == 'A') {
      unsigned char length;
      if (!readExactInput(reinterpret_cast<char *>(&length), 1) || !length)
        return;
      QByteArray package(length, Qt::Uninitialized);
      if (!readExactInput(package.data(), package.size()))
        return;
      const bool knownPackage =
          package == "com.example.calculator" || package == "com.Qtest.App";
      if (mode != "app-timeout")
        report(QString("app-result:A:%1")
                   .arg(mode == "app-unavailable" || !knownPackage
                            ? "unavailable"
                            : "handled"));
      return;
    }
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
    if (QByteArray("Yy").contains(command)) {
      if (mode != "clipboard-timeout")
        report(QString("clipboard-result:%1:%2")
                   .arg(QChar(command), mode == "clipboard-unavailable"
                                            ? "unavailable"
                                            : "handled"));
      return;
    }
    if (QByteArray("Ii").contains(command)) {
      if (mode != "fps-timeout") {
        const bool handled = mode != "fps-unavailable";
        if (handled)
          report(command == 'I' ? "fps-state:started" : "fps-state:stopped");
        report(QString("fps-result:%1:%2")
                   .arg(QChar(command), handled ? "handled" : "unavailable"));
        if (command == 'I' && handled)
          report(mode == "fps-zero" ? "fps-sample:0:0" : "fps-sample:58:2");
      }
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
