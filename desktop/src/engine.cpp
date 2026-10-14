#include "engine.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
#include <QUuid>

const QList<SessionOption> &sessionOptions() {
  static const QList<SessionOption> options{
      {"video-source",
       "Camera",
       "Video source",
       "Mirror the Android display, or stream a phone camera on Android 12+. "
       "Camera sessions do not accept keyboard, mouse or gamepad input.",
       "display",
       {"display", "camera"}},
      {"camera-id",
       "Camera",
       "Exact camera ID",
       "Optional ID from the camera report. When set, it takes priority over "
       "camera facing.",
       QString(""),
       {}},
      {"camera-facing",
       "Camera",
       "Preferred camera facing",
       "Choose front, back or external when no exact camera ID is set. Auto "
       "lets the engine choose.",
       "auto",
       {"auto", "front", "back", "external"}},
      {"camera-size",
       "Camera",
       "Exact camera size",
       "Optional WIDTHxHEIGHT from the camera report. When set, it replaces "
       "the general resolution limit and takes priority over aspect ratio.",
       QString(""),
       {}},
      {"camera-ar",
       "Camera",
       "Camera aspect ratio",
       "Optional ratio such as 16:9, 1.7778 or sensor. Inactive when an exact "
       "camera size is set.",
       QString(""),
       {}},
      {"camera-fps",
       "Camera",
       "Camera frame rate",
       "Zero lets the camera choose. High-speed capture requires an exact "
       "supported frame rate from the camera report.",
       0,
       {},
       0,
       1000},
      {"camera-high-speed",
       "Camera",
       "High-speed camera capture",
       "Requires an exact supported size and frame rate combination from the "
       "camera report.",
       false,
       {}},
      {"camera-torch",
       "Camera",
       "Start with torch enabled",
       "Turn on the selected camera's torch when the session starts, if the "
       "camera supports it.",
       false,
       {}},
      {"camera-zoom",
       "Camera",
       "Initial camera zoom",
       "Optional positive zoom factor such as 2 or 1.5. Supported ranges vary "
       "by camera.",
       QString(""),
       {}},
      {"virtual-display",
       "Virtual display",
       "Create a virtual display",
       "Android 10+. Creates a separate display on the phone when you start a "
       "session, including in read-only mode. It is removed on exit.",
       false,
       {}},
      {"new-display",
       "Virtual display",
       "Resolution and density",
       "Optional WIDTHxHEIGHT/DPI, for example 1920x1080/240. Empty uses phone "
       "defaults, or 1280x960/160 with flexible sizing. /240 changes density "
       "only. The video resolution limit still applies. Requires "
       "virtual-display mode.",
       QString(""),
       {}},
      {"flex-display",
       "Virtual display",
       "Resize Android with the window",
       "Continuously resize the virtual display. Requires control enabled and "
       "no capture crop. Only active in virtual-display mode.",
       false,
       {}},
      {"start-app",
       "Virtual display",
       "Start Android package",
       "Optional exact package, for example com.android.settings. Some phones "
       "have no launcher on virtual displays; starting an app provides "
       "content. Inactive outside virtual-display mode or in read-only mode; "
       "no force-stop is performed.",
       QString(""),
       {}},
      {"display-ime-policy",
       "Virtual display",
       "On-screen keyboard location",
       "Android 10+, secondary or virtual display only. Default leaves policy "
       "to the engine; local shows the keyboard there, fallback uses the "
       "primary display, hide suppresses it.",
       "default",
       {"default", "local", "fallback", "hide"}},
      {"no-vd-destroy-content",
       "Virtual display",
       "Move apps to the primary display on close",
       "With virtual-display mode enabled, move running apps to the primary "
       "display instead of destroying them when the virtual display closes.",
       false,
       {}},
      {"no-vd-system-decorations",
       "Virtual display",
       "Hide virtual-display system decorations",
       "May remove the launcher. Without a started app, the display may stay "
       "empty and produce no video frames. Only active in virtual-display "
       "mode.",
       false,
       {}},
      {"crop",
       "Video",
       "Capture crop",
       "Optional width:height:x:y in the phone's natural orientation, for "
       "example 1080:1920:0:0. The crop must fit the chosen display.",
       QString(""),
       {}},
      {"video-encoder",
       "Video",
       "Video encoder",
       "Leave empty for automatic selection. Inspect the selected phone and "
       "copy an encoder name matching your video codec.",
       QString(""),
       {}},
      {"display-id",
       "Video",
       "Android display ID",
       "Zero is the primary display. Inspect the phone to discover its current "
       "display IDs.",
       0,
       {},
       0,
       2147483647},
      {"capture-orientation",
       "Video",
       "Capture orientation",
       "Affects capture and recordings. Prefix @ locks to natural orientation; "
       "@ alone locks the initial orientation.",
       "0",
       {"0", "90", "180", "270", "flip0", "flip90", "flip180", "flip270", "@",
        "@0", "@90", "@180", "@270", "@flip0", "@flip90", "@flip180",
        "@flip270"}},
      {"no-downsize-on-error",
       "Video",
       "Disable automatic downsizing",
       "Fail rather than retry a lower resolution when the phone encoder "
       "rejects the requested size.",
       false,
       {}},
      {"key-injection",
       "Keyboard",
       "Key injection",
       "SDK keyboard only. Prefer text helps typing; raw events suit games. "
       "Saved but inactive in other input modes.",
       "default",
       {"default", "prefer-text", "raw-key-events"}},
      {"no-key-repeat",
       "Keyboard",
       "Disable key repeat",
       "SDK keyboard only. Do not forward repeated key-down events.",
       false,
       {}},
      {"no-mouse-hover",
       "Mouse",
       "Disable mouse hover",
       "SDK mouse only. Send pointer movement only while clicking or dragging.",
       false,
       {}},
      {"mouse-bind",
       "Mouse",
       "Secondary mouse buttons",
       "Optional primary[:Shift] map for right, middle, 4th and 5th buttons. "
       "Use + to forward, - to ignore, b for Back, h for Home, s for Recents "
       "and n for notifications. Empty uses the selected mouse mode defaults.",
       QString(""),
       {}},
      {"gamepad",
       "Gamepad",
       "Gamepad forwarding",
       "UHID works through Android and may work wirelessly. AOA uses direct "
       "local USB and is unavailable for wireless/remote sessions; disabled "
       "in read-only mode.",
       "disabled",
       {"disabled", "uhid", "aoa"}},
      {"record-format",
       "Recording",
       "Video recording container",
       "MKV is flexible; MP4 is widely supported. MP4 cannot contain raw "
       "audio. Applies to display, virtual-display and camera recording.",
       "mkv",
       {"mkv", "mp4"}},
      {"audio-record-format",
       "Recording",
       "Audio-only recording container",
       "MKA is the flexible audio default. Dedicated OPUS, AAC, FLAC and WAV "
       "files require the matching audio codec. Applies only in Audio-only "
       "mode.",
       "mka",
       {"mka", "m4a", "opus", "aac", "flac", "wav", "mkv", "mp4"}},
      {"record-orientation",
       "Recording",
       "Recording rotation",
       "Rotate the saved video independently of the mirror window. Applies "
       "only when recording.",
       "0",
       {"0", "90", "180", "270"}},
      {"time-limit",
       "Recording",
       "Session time limit (seconds)",
       "Zero means unlimited. Ends the entire session, including mirroring "
       "without recording, through normal cleanup.",
       0,
       {},
       0,
       86400},
      {"video-buffer",
       "Video",
       "Video buffer (ms)",
       "More buffering can smooth playback at the cost of latency.",
       0,
       {},
       0,
       2000},
      {"display-orientation",
       "Video",
       "Display orientation",
       "Rotate the computer display, not the Android device.",
       "0",
       {"0", "90", "180", "270", "flip0", "flip90", "flip180", "flip270"}},
      {"audio-codec",
       "Audio",
       "Audio codec",
       "Requires a compatible encoder on the phone.",
       "opus",
       {"opus", "aac", "flac", "raw"}},
      {"audio-only",
       "Audio",
       "Audio-only session",
       "Listen or record without opening a mirror window. Video and Android "
       "control are disabled for this session.",
       false,
       {}},
      {"audio-source",
       "Audio",
       "Audio source",
       "Auto uses device output for display mirroring and the microphone for "
       "camera capture. Output requires Android 11+. Playback requires Android "
       "13+; apps may opt out.",
       "auto",
       {"auto", "output", "playback", "mic", "mic-unprocessed", "mic-camcorder",
        "mic-voice-recognition", "mic-voice-communication", "voice-call",
        "voice-call-uplink", "voice-call-downlink", "voice-performance"}},
      {"audio-dup",
       "Audio",
       "Keep audio playing on Android",
       "Android 13+. Uses the playback source so captured audio also continues "
       "on the phone. Applications may opt out of playback capture.",
       false,
       {}},
      {"audio-encoder",
       "Audio",
       "Audio encoder",
       "Leave empty for automatic selection. Inspect the phone and choose an "
       "encoder matching the audio codec. Raw audio has no encoder.",
       QString(""),
       {}},
      {"audio-bit-rate",
       "Audio",
       "Audio bit rate",
       "Bits per second; 128000 is the engine default. Ignored for FLAC and "
       "raw audio.",
       128000,
       {},
       1,
       100000000},
      {"require-audio",
       "Audio",
       "Fail if audio capture fails",
       "Normally a session continues with video when Android audio capture is "
       "unavailable. Enable this to fail the whole session instead.",
       false,
       {}},
      {"no-audio-playback",
       "Audio",
       "Do not play audio on this computer",
       "Recordings still include audio. Without a recording, the engine stops "
       "audio capture because it has no remaining consumer.",
       false,
       {}},
      {"audio-buffer",
       "Audio",
       "Audio buffer (ms)",
       "Leave at zero to use the engine default for the selected codec.",
       0,
       {},
       0,
       2000},
      {"audio-output-buffer",
       "Audio",
       "Output buffer (ms)",
       "Computer audio output buffering. Smaller values may cause glitches.",
       10,
       {},
       1,
       1000},
      {"no-control",
       "Control",
       "Read-only session",
       "Disable Android input and APK installation. Screenshots remain "
       "available.",
       false,
       {}},
      {"show-touches",
       "Device",
       "Show physical touches",
       "Shows touches made on the phone, not clicks injected from the "
       "computer.",
       false,
       {}},
      {"power-off-on-close",
       "Device",
       "Turn screen off on close",
       "Turns the display off when the session ends; does not shut down "
       "Android.",
       false,
       {}},
      {"no-clipboard-autosync",
       "Control",
       "Disable clipboard synchronization",
       "Prevent automatic clipboard sharing with the phone.",
       false,
       {}},
      {"legacy-paste",
       "Control",
       "Compatibility paste mode",
       "Inject clipboard text as key events on phones that reject normal "
       "clipboard paste. Non-ASCII text may not survive this fallback.",
       false,
       {}},
      {"push-target",
       "Control",
       "Mirror-window drop destination",
       "Absolute Android directory used when a non-APK file is dropped on "
       "the mirror window. This is separate from DroidCast's explicit file "
       "transfer card.",
       QString("/sdcard/Download/"),
       {}},
      {"shortcut-mod",
       "Shortcuts",
       "Shortcut modifier",
       "One or more comma-separated keys: lctrl, rctrl, lalt, ralt, lsuper "
       "or rsuper. Super is the Windows or Command key.",
       QString("lalt,lsuper"),
       {}},
      {"fullscreen",
       "Window",
       "Start fullscreen",
       "Open the next mirror window fullscreen.",
       false,
       {}},
      {"window-borderless",
       "Window",
       "Borderless mirror",
       "Remove the mirror window decorations.",
       false,
       {}},
      {"disable-screensaver",
       "Window",
       "Keep computer awake",
       "Disable the computer screensaver while mirroring.",
       false,
       {}}};
  return options;
}

bool sessionOptionAvailable(const SessionOption &option, const Preferences &p) {
  const bool readOnly = p.options.value("no-control", false).toBool();
  const bool audioOnly = p.options.value("audio-only", false).toBool();
  const bool camera =
      p.options.value("video-source", "display").toString() == "camera";
  const bool virtualDisplay =
      !camera && p.options.value("virtual-display", false).toBool();
  if (option.key == "audio-only")
    return p.audio;
  if (audioOnly && option.category != "Audio" && option.category != "Recording")
    return false;
  if (option.key == "record-format")
    return !audioOnly;
  if (option.key == "audio-record-format")
    return audioOnly;
  if (audioOnly && option.key == "record-orientation")
    return false;
  if (option.key == "video-source")
    return true;
  if (option.category == "Camera") {
    if (!camera)
      return false;
    if (option.key == "camera-facing")
      return p.options.value("camera-id").toString().trimmed().isEmpty();
    if (option.key == "camera-ar")
      return p.options.value("camera-size").toString().trimmed().isEmpty();
    return true;
  }
  if (option.key == "virtual-display")
    return !camera;
  if (camera && (option.key == "crop" || option.key == "display-id" ||
                 option.key == "display-ime-policy"))
    return false;
  if (option.key == "display-ime-policy")
    return virtualDisplay || p.options.value("display-id", 0).toInt() > 0;
  if (option.category == "Virtual display")
    return virtualDisplay &&
           (!(option.key == "flex-display" || option.key == "start-app") ||
            !readOnly);
  if (option.category == "Keyboard")
    return !camera && !readOnly && p.keyboard == "sdk";
  if (option.category == "Mouse")
    return !camera && !readOnly && p.mouse != "disabled" &&
           (option.key != "no-mouse-hover" || p.mouse == "sdk");
  if (option.category == "Gamepad")
    return !camera && !readOnly;
  if (option.category == "Audio") {
    if (!p.audio)
      return false;
    const auto codec = p.options.value("audio-codec", "opus").toString();
    if (option.key == "audio-encoder")
      return codec != "raw";
    if (option.key == "audio-bit-rate")
      return codec != "raw" && codec != "flac";
    if (option.key == "audio-output-buffer")
      return !p.options.value("no-audio-playback", false).toBool();
    return true;
  }
  return true;
}

QString recordingFormat(const Preferences &p) {
  const bool audioOnly = p.options.value("audio-only", false).toBool();
  const QString key = audioOnly ? "audio-record-format" : "record-format";
  const QString fallback = audioOnly ? "mka" : "mkv";
  const QString format = p.options.value(key, fallback).toString();
  const QStringList allowed =
      audioOnly
          ? QStringList{"mka", "m4a", "opus", "aac", "flac", "wav", "mkv",
                        "mp4"}
          : QStringList{"mkv", "mp4"};
  return allowed.contains(format) ? format : fallback;
}

QVariant normalizedOption(const SessionOption &option, const QVariant &value) {
  if (!value.isValid())
    return option.initial;
  if (!option.choices.isEmpty())
    return option.choices.contains(value.toString()) ? value : option.initial;
  if (option.initial.metaType().id() == QMetaType::Bool)
    return value.toBool();
  if (option.initial.metaType().id() == QMetaType::QString)
    return value.toString().trimmed();
  bool ok = false;
  int number = value.toInt(&ok);
  return ok ? QVariant(qBound(option.minimum, number, option.maximum))
            : option.initial;
}

QString textOptionError(const QString &key, const QString &value) {
  if (value.isEmpty())
    return {};
  if (key == "camera-id") {
    if (value.size() > 128 ||
        !QRegularExpression("\\A[A-Za-z0-9_.:-]+\\z").match(value).hasMatch())
      return "Use a camera ID from the phone report (letters, numbers, dots, "
             "underscores, colons or hyphens), or leave empty.";
  } else if (key == "camera-size") {
    static const QRegularExpression pattern("\\A([0-9]{1,5})x([0-9]{1,5})\\z");
    const auto match = pattern.match(value);
    const bool valid = match.hasMatch() && match.captured(1).toInt() > 0 &&
                       match.captured(1).toInt() <= 65535 &&
                       match.captured(2).toInt() > 0 &&
                       match.captured(2).toInt() <= 65535;
    if (!valid)
      return "Use WIDTHxHEIGHT with values from 1–65535, or leave empty.";
  } else if (key == "camera-ar") {
    bool valid = value == "sensor";
    if (!valid && value.contains(':')) {
      static const QRegularExpression ratio("\\A([0-9]{1,5}):([0-9]{1,5})\\z");
      const auto match = ratio.match(value);
      valid = match.hasMatch() && match.captured(1).toInt() > 0 &&
              match.captured(1).toInt() <= 10000 &&
              match.captured(2).toInt() > 0 &&
              match.captured(2).toInt() <= 10000;
    } else if (!valid) {
      bool ok = false;
      const double aspect = value.toDouble(&ok);
      valid = ok && aspect > 0 && aspect <= 10000 &&
              QRegularExpression("\\A[0-9]+(?:\\.[0-9]+)?\\z")
                  .match(value)
                  .hasMatch();
    }
    if (!valid)
      return "Use sensor, a positive ratio such as 16:9, a positive decimal "
             "such as 1.7778, or leave empty.";
  } else if (key == "camera-zoom") {
    bool ok = false;
    const double zoom = value.toDouble(&ok);
    if (!ok || zoom <= 0 || zoom > 1000 ||
        !QRegularExpression("\\A[0-9]+(?:\\.[0-9]+)?\\z")
             .match(value)
             .hasMatch())
      return "Use a positive zoom factor up to 1000, such as 1.5, or leave "
             "empty.";
  } else if (key == "new-display") {
    static const QRegularExpression pattern(
        "\\A(?:([0-9]{1,5})x([0-9]{1,5}))?(?:/([0-9]{1,5}))?\\z");
    const auto match = pattern.match(value);
    bool valid = match.hasMatch();
    for (int i = 1; valid && i <= 3; ++i)
      if (!match.captured(i).isEmpty())
        valid =
            match.captured(i).toInt() > 0 && match.captured(i).toInt() <= 65535;
    if (!valid)
      return "Use WIDTHxHEIGHT, WIDTHxHEIGHT/DPI, /DPI, or leave empty. Each "
             "number must be 1–65535; device limits may be lower.";
  } else if (key == "start-app") {
    static const QRegularExpression pattern(
        "\\A[A-Za-z][A-Za-z0-9_]*(?:\\.[A-Za-z][A-Za-z0-9_]*)*\\z");
    if (value.toUtf8().size() > 255 || !pattern.match(value).hasMatch())
      return "Enter an exact Android package name such as "
             "com.android.settings, without spaces, search prefixes or "
             "force-stop prefixes.";
  } else if (key == "crop") {
    const auto parts = value.split(':');
    bool valid = parts.size() == 4;
    for (int i = 0; valid && i < parts.size(); ++i) {
      bool ok;
      const auto n = parts[i].toUInt(&ok);
      valid =
          ok &&
          QRegularExpression("\\A[0-9]{1,5}\\z").match(parts[i]).hasMatch() &&
          n <= 65535 && (i >= 2 || n > 0);
    }
    if (!valid)
      return "Use width:height:x:y; dimensions 1–65535 and offsets 0–65535, or "
             "leave empty.";
  } else if ((key == "video-encoder" || key == "audio-encoder") &&
             !QRegularExpression("\\A[A-Za-z0-9_.-]{1,256}\\z")
                  .match(value)
                  .hasMatch()) {
    return "Use an encoder name from the phone report (letters, numbers, "
           "dots, underscores or hyphens), or leave empty.";
  } else if (key == "push-target") {
    const auto parts = value.split('/', Qt::KeepEmptyParts);
    if (value.isEmpty() || value.size() > 256 || !value.startsWith('/') ||
        QDir::cleanPath(value) == "/" || parts.contains(".") ||
        parts.contains("..") ||
        QRegularExpression("[\\x00-\\x1f\\x7f]").match(value).hasMatch())
      return "Use an absolute Android directory other than /, without . or "
             ".. components.";
  } else if (key == "shortcut-mod") {
    const auto keys = value.split(',', Qt::KeepEmptyParts);
    const QSet<QString> allowed{"lctrl", "rctrl",  "lalt",
                                "ralt",  "lsuper", "rsuper"};
    QSet<QString> unique;
    for (const auto &modifier : keys)
      if (!allowed.contains(modifier) || unique.contains(modifier))
        return "Use unique comma-separated modifiers: lctrl, rctrl, lalt, "
               "ralt, lsuper or rsuper.";
      else
        unique.insert(modifier);
    if (keys.isEmpty() || keys.size() > allowed.size())
      return "Choose at least one supported shortcut modifier.";
  } else if (key == "mouse-bind" &&
             !QRegularExpression("\\A[+bhsn-]{4}(?::[+bhsn-]{4})?\\z")
                  .match(value)
                  .hasMatch()) {
    return "Use four actions, optionally followed by : and four Shift "
           "actions. Allowed characters: + - b h s n.";
  }
  return {};
}

PreferenceIssue sessionIssue(const Preferences &p, bool recording) {
  const auto transportIssue = connectionIssue(p);
  if (!transportIssue.isEmpty())
    return {"connection", transportIssue};
  for (const auto &option : sessionOptions()) {
    if (!sessionOptionAvailable(option, p))
      continue;
    const auto error = textOptionError(
        option.key,
        normalizedOption(option, p.options.value(option.key)).toString());
    if (!error.isEmpty())
      return {option.key, option.title + ": " + error};
  }
  const bool camera =
      p.options.value("video-source", "display").toString() == "camera";
  const bool audioOnly = p.options.value("audio-only", false).toBool();
  if (audioOnly && !p.audio)
    return {"audio-only", "Audio-only sessions require audio forwarding."};
  if (audioOnly && p.options.value("no-audio-playback", false).toBool() &&
      !recording)
    return {"no-audio-playback",
            "Audio-only needs computer playback or a recording destination."};
  const bool virtualDisplay =
      !camera && p.options.value("virtual-display", false).toBool();
  if (camera && p.options.value("camera-high-speed", false).toBool() &&
      p.options.value("camera-fps", 0).toInt() == 0)
    return {"camera-fps",
            "High-speed camera capture needs an explicit supported camera "
            "frame rate."};
  if (virtualDisplay && p.options.value("display-id", 0).toInt() != 0)
    return {"display-id",
            "A new virtual display cannot use an existing display ID. Set "
            "Android display ID to 0 or disable virtual-display mode."};
  if (virtualDisplay && p.options.value("flex-display", false).toBool() &&
      !p.options.value("no-control", false).toBool() &&
      !p.options.value("crop").toString().trimmed().isEmpty())
    return {"crop",
            "Resizable virtual displays cannot use a capture crop. Clear "
            "Capture crop or disable Resize Android with the window."};
  if (p.audio && p.options.value("audio-dup", false).toBool() &&
      p.options.value("audio-source", "auto").toString() != "playback")
    return {"audio-source",
            "Keeping audio on Android requires the playback audio source."};
  if (recording && !audioOnly && recordingFormat(p) == "mp4" && p.audio &&
      p.options.value("audio-codec").toString() == "raw")
    return {"audio-codec", "MP4 cannot contain raw audio. Choose MKV, another "
                           "audio codec, or disable audio forwarding."};
  if (recording && audioOnly) {
    const auto format = recordingFormat(p);
    const auto codec =
        p.options.value("audio-codec", "opus").toString().toLower();
    if ((format == "mp4" || format == "m4a") && codec == "raw")
      return {"audio-record-format",
              "MP4/M4A cannot contain raw audio. Choose WAV, MKA/MKV, or an "
              "encoded audio codec."};
    const QMap<QString, QString> required{{"opus", "opus"}, {"aac", "aac"},
                                          {"flac", "flac"}, {"wav", "raw"}};
    if (required.contains(format) && required.value(format) != codec)
      return {"audio-record-format",
              format.toUpper() + " recording requires the " +
                  required.value(format).toUpper() +
                  " audio codec. Change the codec or container."};
  }
  return {};
}

Preferences Preferences::load(QSettings &s) {
  Preferences p = bundledPreferences();
  const int settingsVersion = s.value("metadata/settings-version", 1).toInt();
  p.size = qBound(0, s.value("session/size", p.size).toInt(), 8192);
  p.fps = qBound(1, s.value("session/fps", p.fps).toInt(), 240);
  p.bitrate = qBound(1, s.value("session/bitrate", p.bitrate).toInt(), 200);
  p.keyboard = s.value("session/keyboard", p.keyboard).toString();
  if (p.keyboard != "sdk" && p.keyboard != "uhid" && p.keyboard != "aoa" &&
      p.keyboard != "disabled")
    p.keyboard = "sdk";
  p.mouse = s.value("session/mouse", p.mouse).toString();
  if (p.mouse != "sdk" && p.mouse != "uhid" && p.mouse != "aoa" &&
      p.mouse != "disabled")
    p.mouse = "sdk";
  p.codec = s.value("session/codec", p.codec).toString();
  if (!QStringList{"h264", "h265", "av1"}.contains(p.codec))
    p.codec = "h264";
  p.mediaDirectory = s.value("captures/directory", p.mediaDirectory).toString();
  p.audio = s.value("session/audio", p.audio).toBool();
  p.awake = s.value("session/awake", p.awake).toBool();
  p.screenOff = s.value("session/screenOff", p.screenOff).toBool();
  p.top = s.value("session/top", p.top).toBool();
  p.remoteAdb = s.value("connection/remote-adb", false).toBool();
  p.adbServerEndpoint =
      s.value("connection/adb-server").toString().trimmed().left(512);
  p.forceAdbForward = s.value("connection/force-adb-forward", false).toBool();
  p.tunnelHost =
      s.value("connection/tunnel-host").toString().trimmed().left(15);
  p.tunnelPort = qBound(0, s.value("connection/tunnel-port", 0).toInt(), 65535);
  p.otgSerial =
      s.value("otg/serial").toString().trimmed().left(256);
  p.otgKeyboard = s.value("otg/keyboard", true).toBool();
  p.otgMouse = s.value("otg/mouse", true).toBool();
  p.otgGamepad = s.value("otg/gamepad", false).toBool();
  if (!p.otgKeyboard && !p.otgMouse && !p.otgGamepad) {
    p.otgKeyboard = true;
    p.otgMouse = true;
  }
  for (const auto &option : sessionOptions())
    p.options.insert(
        option.key, normalizedOption(option, s.value("options/" + option.key)));
  // Earlier releases wrote "output" for everyone because it was the default,
  // so it cannot represent an intentional override. Migrate it to scrcpy's
  // source-aware default before camera capture becomes available.
  if (settingsVersion < 2 &&
      p.options.value("audio-source").toString() == "output")
    p.options.insert("audio-source", "auto");
  if (p.options.value("audio-dup", false).toBool())
    p.options.insert("audio-source", "playback");
  if (settingsVersion < 3) {
    const auto legacy = p.options.value("record-format", "mkv").toString();
    p.options.insert("audio-record-format",
                     QStringList{"mkv", "mp4"}.contains(legacy) ? legacy
                                                                : "mka");
  }
  return p;
}

void Preferences::save(QSettings &s) const {
  s.setValue("metadata/settings-version", 4);
  s.setValue("session/codec", codec);
  s.setValue("session/mouse", mouse);
  s.setValue("captures/directory", mediaDirectory);
  s.setValue("session/size", size);
  s.setValue("session/fps", fps);
  s.setValue("session/bitrate", bitrate);
  s.setValue("session/keyboard", keyboard);
  s.setValue("session/audio", audio);
  s.setValue("session/awake", awake);
  s.setValue("session/screenOff", screenOff);
  s.setValue("session/top", top);
  s.setValue("connection/remote-adb", remoteAdb);
  s.setValue("connection/adb-server", adbServerEndpoint);
  s.setValue("connection/force-adb-forward", forceAdbForward);
  s.setValue("connection/tunnel-host", tunnelHost);
  s.setValue("connection/tunnel-port", tunnelPort);
  s.setValue("otg/serial", otgSerial);
  s.setValue("otg/keyboard", otgKeyboard);
  s.setValue("otg/mouse", otgMouse);
  s.setValue("otg/gamepad", otgGamepad);
  for (const auto &option : sessionOptions())
    s.setValue("options/" + option.key,
               normalizedOption(option, options.value(option.key)));
}

QList<Device> parseDevices(const QString &output) {
  QList<Device> devices;
  for (const auto &line : output.split('\n')) {
    const auto fields =
        line.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (fields.size() < 2 || line.startsWith("List ") || line.startsWith('*'))
      continue;
    const auto state = fields[1];
    if (state != "device" && state != "offline" && state != "unauthorized" &&
        state != "no" && state != "recovery" && state != "sideload" &&
        state != "bootloader")
      continue;
    Device d{fields[0], state == "no" ? "no permissions" : state, fields[0],
             "USB"};
    if (d.serial.contains(':') || d.serial.contains("_adb-tls-connect"))
      d.connection = "Wireless";
    else if (d.serial.startsWith("emulator-"))
      d.connection = "Emulator";
    for (const auto &field : fields) {
      if (field.startsWith("model:"))
        d.model = field.mid(6).replace('_', ' ');
    }
    devices.append(d);
  }
  return devices;
}

QList<OtgDevice> parseOtgDevices(const QString &output) {
  QList<OtgDevice> devices;
  constexpr auto prefix = "DROIDCAST_USB/1 ";
  constexpr qsizetype prefixLength = sizeof("DROIDCAST_USB/1 ") - 1;
  for (const auto &line : output.split('\n')) {
    if (!line.startsWith(prefix))
      continue;
    const auto fields = line.mid(prefixLength).split('\t');
    if (fields.size() != 4 || fields[0].isEmpty() || fields[0].size() > 256 ||
        !fields[1].contains(
            QRegularExpression("^[0-9a-fA-F]{4}:[0-9a-fA-F]{4}$")))
      continue;
    devices.append({fields[0], fields[1], fields[2], fields[3]});
  }
  return devices;
}

QList<CameraCapability> parseCameraCapabilities(const QString &output) {
  QList<CameraCapability> cameras;
  bool inCameraList = false, highSpeed = false;
  static const QRegularExpression cameraPattern(
      "\\A\\s*--camera-id=([^\\s]+)\\s+\\(([^,]+),\\s*([0-9]+x[0-9]+)"
      "(?:,\\s*fps=\\{([^}]*)\\})?(?:,\\s*zoom-range=(\\[[^]]+\\]))?\\)\\s*"
      "\\z");
  static const QRegularExpression sizePattern(
      "\\A\\s*-\\s*([0-9]+x[0-9]+)(?:\\s+\\(fps=\\{([^}]*)\\}\\))?\\s*\\z");
  auto rates = [](const QString &text) {
    QStringList result;
    for (const auto &rate : text.split(',', Qt::SkipEmptyParts)) {
      const auto trimmed = rate.trimmed();
      bool ok = false;
      const int value = trimmed.toInt(&ok);
      if (ok && value > 0 && !result.contains(QString::number(value)))
        result.append(QString::number(value));
    }
    return result;
  };
  for (const auto &raw : output.split('\n')) {
    const auto line = raw.trimmed();
    if (line.contains("List of cameras:")) {
      inCameraList = true;
      highSpeed = false;
      continue;
    }
    if (!inCameraList)
      continue;
    if (line.startsWith("List of "))
      break;
    const auto camera = cameraPattern.match(raw);
    if (camera.hasMatch()) {
      cameras.append({camera.captured(1),
                      camera.captured(2).trimmed(),
                      camera.captured(3),
                      camera.captured(5),
                      rates(camera.captured(4)),
                      {},
                      {}});
      highSpeed = false;
      continue;
    }
    if (line.startsWith("High speed capture")) {
      highSpeed = true;
      continue;
    }
    if (cameras.isEmpty())
      continue;
    const auto size = sizePattern.match(raw);
    if (!size.hasMatch())
      continue;
    if (highSpeed)
      cameras.last().highSpeedFrameRates.insert(size.captured(1),
                                                rates(size.captured(2)));
    else if (!cameras.last().sizes.contains(size.captured(1)))
      cameras.last().sizes.append(size.captured(1));
  }
  return cameras;
}

QList<DisplayCapability> parseDisplayCapabilities(const QString &output) {
  QList<DisplayCapability> result;
  bool inList = false;
  static const QRegularExpression pattern(
      "\\A\\s*--display-id=([0-9]+)\\s+\\(([^)]*)\\)\\s*\\z");
  for (const auto &raw : output.split('\n')) {
    const auto line = raw.trimmed();
    if (line.contains("List of displays:")) {
      inList = true;
      continue;
    }
    if (!inList)
      continue;
    if (line.startsWith("List of "))
      break;
    const auto match = pattern.match(raw);
    if (!match.hasMatch())
      continue;
    bool ok = false;
    const int id = match.captured(1).toInt(&ok);
    if (ok)
      result.append({id, match.captured(2).trimmed()});
  }
  return result;
}

QList<VideoEncoderCapability>
parseVideoEncoderCapabilities(const QString &output) {
  QList<VideoEncoderCapability> result;
  bool inList = false;
  static const QRegularExpression pattern(
      "\\A\\s*--video-codec=(h264|h265|av1)\\s+"
      "--video-encoder=([^\\s]+)(?:\\s+(.*))?\\z");
  for (const auto &raw : output.split('\n')) {
    const auto line = raw.trimmed();
    if (line.contains("List of video encoders:")) {
      inList = true;
      continue;
    }
    if (!inList)
      continue;
    if (line.startsWith("List of "))
      break;
    const auto match = pattern.match(raw);
    if (match.hasMatch())
      result.append(
          {match.captured(1), match.captured(2), match.captured(3).trimmed()});
  }
  return result;
}

QList<AudioEncoderCapability>
parseAudioEncoderCapabilities(const QString &output) {
  QList<AudioEncoderCapability> result;
  bool inList = false;
  static const QRegularExpression pattern(
      "\\A\\s*--audio-codec=(opus|aac|flac)\\s+"
      "--audio-encoder=([^\\s]+)(?:\\s+(.*))?\\z");
  for (const auto &raw : output.split('\n')) {
    const auto line = raw.trimmed();
    if (line.contains("List of audio encoders:")) {
      inList = true;
      continue;
    }
    if (!inList)
      continue;
    if (line.startsWith("List of "))
      break;
    const auto match = pattern.match(raw);
    if (match.hasMatch())
      result.append(
          {match.captured(1), match.captured(2), match.captured(3).trimmed()});
  }
  return result;
}

QList<AppCapability> parseAppCapabilities(const QString &output) {
  QList<AppCapability> result;
  bool inList = false, pendingSystem = false;
  QString pendingName;
  static const QRegularExpression complete(
      "\\A\\s*([*-])\\s+(.+?)\\s{2,}([A-Za-z][A-Za-z0-9_]*(?:\\.[A-Za-z][A-Za-"
      "z0-9_]*)*)\\s*\\z");
  static const QRegularExpression package(
      "\\A[A-Za-z][A-Za-z0-9_]*(?:\\.[A-Za-z][A-Za-z0-9_]*)*\\z");
  for (const auto &raw : output.split('\n')) {
    const auto line = raw.trimmed();
    if (line.contains("List of apps:")) {
      inList = true;
      continue;
    }
    if (!inList)
      continue;
    if (line.startsWith("List of "))
      break;
    const auto match = complete.match(raw);
    if (match.hasMatch()) {
      result.append({match.captured(2).trimmed(), match.captured(3),
                     match.captured(1) == "*"});
      pendingName.clear();
    } else if (!pendingName.isEmpty() && package.match(line).hasMatch()) {
      result.append({pendingName, line, pendingSystem});
      pendingName.clear();
    } else if (line.startsWith("* ") || line.startsWith("- ")) {
      pendingSystem = line.startsWith("* ");
      pendingName = line.mid(2).trimmed();
    }
  }
  return result;
}

QStringList mirrorArguments(const QString &serial, const Preferences &p,
                            const QString &recording) {
  const bool audioOnly = p.options.value("audio-only", false).toBool();
  const bool camera =
      p.options.value("video-source", "display").toString() == "camera";
  const auto cameraSize = p.options.value("camera-size").toString().trimmed();
  const bool exactCameraSize =
      camera && cameraSize.isEmpty() == false &&
      textOptionError("camera-size", cameraSize).isEmpty();
  QStringList args{"--serial=" + serial,
                   "--window-title=DroidCast Desktop | " + serial};
  if (audioOnly)
    args << "--no-video" << "--no-control";
  else {
    args << "--max-fps=" + QString::number(p.fps)
         << "--video-bit-rate=" + QString::number(p.bitrate) + "M"
         << "--keyboard=" + p.keyboard << "--mouse=" + p.mouse
         << "--video-codec=" + p.codec;
    if (p.size > 0 && !exactCameraSize)
      args << "--max-size=" + QString::number(p.size);
  }
  if (!p.audio)
    args << "--no-audio";
  if (p.awake)
    args << "--stay-awake";
  if (p.screenOff)
    args << "--turn-screen-off";
  if (p.top)
    args << "--always-on-top";
  if (!recording.isEmpty())
    args << "--record=" + recording;
  const bool readOnly =
      audioOnly || p.options.value("no-control", false).toBool();
  if (readOnly) {
    args.removeAll("--stay-awake");
    args.removeAll("--turn-screen-off");
  }
  for (const auto &option : sessionOptions()) {
    const auto value = normalizedOption(option, p.options.value(option.key));
    if (!textOptionError(option.key, value.toString()).isEmpty())
      continue;
    if (!sessionOptionAvailable(option, p))
      continue;
    if (option.key == "virtual-display")
      continue;
    if (option.key == "audio-only")
      continue;
    if (option.key == "camera-facing" &&
        !p.options.value("camera-id").toString().trimmed().isEmpty())
      continue;
    if (option.key == "camera-ar" && exactCameraSize)
      continue;
    if (option.key == "new-display") {
      args << (value.toString().isEmpty()
                   ? "--new-display"
                   : "--new-display=" + value.toString());
      continue;
    }
    if (option.key == "record-format" ||
        option.key == "audio-record-format") {
      if (!recording.isEmpty())
        args << "--record-format=" + recordingFormat(p);
      continue;
    }
    if (option.key == "record-orientation" && recording.isEmpty())
      continue;
    if (option.key == "key-injection") {
      if (value != option.initial)
        args << "--" + value.toString();
      continue;
    }
    if (value == option.initial)
      continue;
    if (!p.audio && option.category == "Audio")
      continue;
    if (readOnly &&
        (option.key == "show-touches" || option.key == "power-off-on-close"))
      continue;
    if (option.initial.metaType().id() == QMetaType::Bool) {
      if (value.toBool())
        args << "--" + option.key;
    } else
      args << "--" + option.key + "=" + value.toString();
  }
  if (p.remoteAdb) {
    if (p.forceAdbForward)
      args << "--force-adb-forward";
    if (!p.tunnelHost.isEmpty())
      args << "--tunnel-host=" + p.tunnelHost;
    if (p.tunnelPort > 0)
      args << "--tunnel-port=" + QString::number(p.tunnelPort);
  }
  return args;
}

bool validEndpoint(const QString &endpoint) {
  // Require an explicit port: Android's pairing and connection ports differ.
  static const QRegularExpression pattern(
      "\\A(?:[A-Za-z0-9](?:[A-Za-z0-9.-]*[A-Za-z0-9])?|\\[[0-9A-Fa-f:]+\\]):(["
      "0-9]{1,5})\\z");
  auto match = pattern.match(endpoint);
  return match.hasMatch() && match.captured(1).toInt() > 0 &&
         match.captured(1).toInt() <= 65535;
}

bool validIpv4Address(const QString &address) {
  const auto parts = address.split('.');
  if (parts.size() != 4)
    return false;
  for (const auto &part : parts) {
    bool ok = false;
    const int value = part.toInt(&ok);
    if (!ok || value < 0 || value > 255 || part.isEmpty() ||
        (part.size() > 1 && part.startsWith('0')))
      return false;
  }
  return true;
}

QString connectionIssue(const Preferences &p) {
  if (!p.remoteAdb)
    return {};
  if (!validEndpoint(p.adbServerEndpoint))
    return "Enter the remote ADB server as host:port.";
  if (!p.tunnelHost.isEmpty() && !validIpv4Address(p.tunnelHost))
    return "Tunnel host must be an IPv4 address accepted by scrcpy.";
  if (p.tunnelPort < 0 || p.tunnelPort > 65535)
    return "Tunnel port must be zero or between 1 and 65535.";
  return {};
}

QString adbServerSocket(const Preferences &p) {
  return p.remoteAdb && validEndpoint(p.adbServerEndpoint)
             ? "tcp:" + p.adbServerEndpoint
             : QString{};
}

QString executablePath(const QString &program) {
  if (program.isEmpty())
    return {};
  return QStandardPaths::findExecutable(program);
}

QString runtimeDirectory() {
  // Build/test override only; never exposed as an end-user setting.
  return qEnvironmentVariable("DROIDCAST_RUNTIME_DIR",
                              QCoreApplication::applicationDirPath() +
                                  "/runtime");
}

Preferences bundledPreferences() {
  Preferences p;
  const QDir runtime(runtimeDirectory());
#ifdef Q_OS_WIN
  p.adb = runtime.filePath("adb.exe");
  p.scrcpy = runtime.filePath("scrcpy.exe");
#else
  p.adb = runtime.filePath("adb");
  p.scrcpy = runtime.filePath("scrcpy");
#endif
  p.server = runtime.filePath("scrcpy-server");
  p.mediaDirectory =
      QDir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))
          .filePath("DroidCast");
  return p;
}

Engine::Engine(QObject *parent) : QObject(parent) {
  monitorRetry.setSingleShot(true);
  monitorStartupTimeout.setSingleShot(true);
  monitorStopTimeout.setSingleShot(true);
  connect(&monitorRetry, &QTimer::timeout, this, &Engine::launchDeviceMonitor);
  connect(&monitorStartupTimeout, &QTimer::timeout, this, [this] {
    failDeviceMonitor("ADB device events did not initialize. Retrying the subscription.");
  });
  connect(&monitorStopTimeout, &QTimer::timeout, &deviceMonitor, &QProcess::kill);
  connect(&deviceMonitor, &QProcess::stateChanged, this, [this] { emit deviceMonitoringChanged(); });
  connect(&deviceMonitor, &QProcess::readyReadStandardOutput, this, &Engine::readDeviceMonitor);
  connect(&deviceMonitor, &QProcess::readyReadStandardError, this, [this] {
    emit log(QString::fromUtf8(deviceMonitor.readAllStandardError().left(8192)));
  });
  connect(&deviceMonitor, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart && monitorEnabled)
      failDeviceMonitor("Could not start ADB device events. Check the bundled runtime.");
  });
  connect(&deviceMonitor, &QProcess::finished, this, [this] {
    monitorStartupTimeout.stop(); monitorStopTimeout.stop();
    if (!monitorEnabled) return;
    if (monitorRestart) {
      monitorRestart = false;
      launchDeviceMonitor();
    } else {
      if (!monitorFailed) failDeviceMonitor("ADB device events disconnected. Retrying the subscription.");
      else scheduleMonitorRetry();
    }
  });
  connect(this, &Engine::sessionChanged, this, [this] {
    if (!monitorEnabled) return;
    if (otgSession()) {
      if (deviceMonitorRunning() && !monitorRestart) restartDeviceMonitor();
    } else if (!deviceMonitorRunning() && !monitorRetry.isActive()) {
      launchDeviceMonitor();
    } else if (monitorSocket != processEnvironment(false).value("ADB_SERVER_SOCKET")) {
      restartDeviceMonitor();
    }
  });
  inspectionTimeout.setSingleShot(true);
  otgInspectionTimeout.setSingleShot(true);
  inspector.setProcessChannelMode(QProcess::MergedChannels);
  connect(&inspectionTimeout, &QTimer::timeout, this, [this] {
    inspectionAborted = true;
    inspector.kill();
    emit inspectionResult(inspectionSerial,
                          "Inspection timed out. Unlock the phone, check the "
                          "connection and retry.");
  });
  connect(&inspector, &QProcess::stateChanged, this,
          [this] { emit inspectionChanged(); });
  connect(&inspector, &QProcess::readyReadStandardOutput, this, [this] {
    inspectionOutput += inspector.readAllStandardOutput();
    if (inspectionOutput.size() > 1024 * 1024 && !inspectionAborted) {
      inspectionAborted = true;
      inspector.kill();
      emit inspectionResult(inspectionSerial,
                            "Inspection output exceeded the safety limit. "
                            "Reconnect the phone and retry.");
    }
  });
  connect(&inspector, &QProcess::errorOccurred, this,
          [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
              inspectionTimeout.stop();
              inspectionAborted = true;
              emit inspectionResult(
                  inspectionSerial,
                  "Could not start the bundled inspection engine. Reinstall "
                  "the complete app bundle.");
            }
          });
  connect(&inspector, &QProcess::finished, this,
          [this](int code, QProcess::ExitStatus status) {
            inspectionTimeout.stop();
            if (inspectionAborted)
              return;
            inspectionOutput += inspector.readAllStandardOutput();
            if (inspectionOutput.size() > 1024 * 1024) {
              emit inspectionResult(
                  inspectionSerial,
                  "Inspection output exceeded the safety limit.");
              return;
            }
            const auto output = QString::fromUtf8(inspectionOutput).trimmed();
            emit inspectionResult(inspectionSerial,
                                  code == 0 && status == QProcess::NormalExit &&
                                          !output.isEmpty()
                                      ? output
                                      : "Inspection failed. Reconnect or "
                                        "authorize the phone and retry.\n\n" +
                                            output);
          });
  usbInspector.setProcessChannelMode(QProcess::MergedChannels);
  connect(&usbInspector, &QProcess::stateChanged, this,
          [this] { emit otgDevicesChanged(); });
  connect(&usbInspector, &QProcess::readyReadStandardOutput, this, [this] {
    otgInspectionOutput += usbInspector.readAllStandardOutput();
    if (otgInspectionOutput.size() > 128 * 1024)
      usbInspector.kill();
  });
  connect(&otgInspectionTimeout, &QTimer::timeout, this,
          [this] { usbInspector.kill(); });
  connect(&usbInspector, &QProcess::finished, this,
          [this](int code, QProcess::ExitStatus status) {
            otgInspectionTimeout.stop();
            otgInspectionOutput += usbInspector.readAllStandardOutput();
            if (code == 0 && status == QProcess::NormalExit &&
                otgInspectionOutput.size() <= 128 * 1024) {
              otgDevices =
                  parseOtgDevices(QString::fromUtf8(otgInspectionOutput));
              emit message(otgDevices.isEmpty()
                               ? "No accessible USB devices were found for USB control."
                               : "Eligible USB devices were refreshed.");
            } else {
              otgDevices.clear();
              emit message("Could not list USB devices. Check the cable and "
                           "USB permissions.");
            }
            emit otgDevicesChanged();
          });
  connect(&usbInspector, &QProcess::errorOccurred, this,
          [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart)
              emit message("Could not start the bundled USB device inspector. "
                           "Reinstall DroidCast Desktop.");
          });
  bridgeCommandTimeout.setSingleShot(true);
  connect(&bridgeCommandTimeout, &QTimer::timeout, this, [this] {
    const bool cameraCommand =
        QByteArray("Tt+-").contains(pendingBridgeCommand);
    const bool androidCommand =
        QByteArray("01NSCDV").contains(pendingBridgeCommand);
    const bool clipboardCommand =
        QByteArray("Yy").contains(pendingBridgeCommand);
    const bool fpsCommand = QByteArray("Ii").contains(pendingBridgeCommand);
    const bool appCommand = pendingBridgeCommand == 'A';
    pendingBridgeCommand = 0;
    if (cameraCommand) {
      cameraCommandsHealthy = false;
      emit cameraControlMessage(
          "No confirmation from the camera. Restart the session to re-enable "
          "camera controls; no command was retried.");
    } else if (androidCommand) {
      androidCommandsHealthy = false;
      emit androidControlMessage(
          "No confirmation from Android. Restart the session to re-enable "
          "device actions; no command was retried.");
    } else if (clipboardCommand) {
      clipboardCommandsHealthy = false;
      emit clipboardControlMessage(
          "No clipboard confirmation arrived. Restart the session to "
          "re-enable clipboard actions; no request was retried.");
    } else if (fpsCommand) {
      fpsCommandsHealthy = false;
      fpsMeasurementActive = false;
      emit fpsControlMessage(
          "No FPS confirmation arrived. Restart the session to re-enable "
          "measurement controls; no request was retried.");
    } else if (appCommand) {
      appCommandsHealthy = false;
      emit appLaunchMessage(
          "No app-launch confirmation arrived. Restart the session to "
          "re-enable launching; no request was retried.");
    } else {
      windowCommandsHealthy = false;
      emit windowControlMessage(
          "No confirmation from the mirror. Restart the session to re-enable "
          "window controls; no command was retried.");
    }
    emit windowControlsChanged();
    emit cameraControlsChanged();
    emit androidControlsChanged();
    emit clipboardControlsChanged();
    emit fpsControlsChanged();
    emit appLaunchControlsChanged();
  });
  scanTimeout.setSingleShot(true);
  wirelessTimeout.setSingleShot(true);
  stopTimeout.setSingleShot(true);
  audioCapabilityTimeout.setSingleShot(true);
  otgCapabilityTimeout.setSingleShot(true);
  connect(&scanTimeout, &QTimer::timeout, this, [this] {
    scanTimedOut = true;
    scan.kill();
    emit message("Device scan timed out. Check ADB and retry.");
  });
  connect(&wirelessTimeout, &QTimer::timeout, this, [this] {
    wirelessTimedOut = true;
    wireless.kill();
    emit message(
        "Wireless request timed out. Check the address and phone, then retry.");
  });
  connect(&stopTimeout, &QTimer::timeout, this, [this] {
    if (running()) {
      forcedStop = true;
      emit message(activeOtg
                       ? "USB control did not exit; forcing it to stop."
                       : "scrcpy did not exit; forcing it to stop. A "
                         "recording may be incomplete.");
      mirror.kill();
    }
  });
  connect(&audioCapabilityTimeout, &QTimer::timeout, this, [this] {
    if (!running() || !activeAudioOnly || audioLifecycleReady)
      return;
    audioLifecycleFailure = true;
    state = SessionState::Failed;
    stopping = true;
    emit message("The bundled engine does not support audio-only lifecycle "
                 "reporting. Reinstall this DroidCast build.");
    mirror.write("Q", 1);
    stopTimeout.start(5000);
    emit sessionChanged();
  });
  connect(&otgCapabilityTimeout, &QTimer::timeout, this, [this] {
    if (!running() || !activeOtg || state != SessionState::Starting)
      return;
    state = SessionState::Failed;
    stopping = true;
    otgLifecycleFailure = true;
    emit message("The bundled engine did not confirm USB control readiness. "
                 "Reinstall this DroidCast build.");
    mirror.write("Q", 1);
    stopTimeout.start(5000);
    emit sessionChanged();
  });
  connect(&scan, &QProcess::stateChanged, this, [this] { emit scanChanged(); });
  connect(&scan, &QProcess::readyReadStandardOutput, this,
          [this] { scanOutput += scan.readAllStandardOutput(); });
  connect(&scan, &QProcess::readyReadStandardError, this,
          [this] { emit log(QString::fromUtf8(scan.readAllStandardError())); });
  connect(&scan, &QProcess::errorOccurred, this,
          [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
              scanTimeout.stop();
              if (monitorEnabled) return;
              devices.clear();
              emit devicesChanged();
              emit message("The bundled connection service could not start. "
                           "See Diagnostics or reinstall DroidCast Desktop.");
              emit deviceScanCompleted(false);
            }
          });
  connect(&scan, &QProcess::finished, this,
          [this](int code, QProcess::ExitStatus status) {
            scanTimeout.stop();
            scanOutput += scan.readAllStandardOutput();
            if (monitorEnabled) return;
            const bool success =
                code == 0 && status == QProcess::NormalExit && !scanTimedOut;
            devices = success ? parseDevices(QString::fromUtf8(scanOutput))
                              : QList<Device>{};
            emit devicesChanged();
            emit deviceScanCompleted(success);
            if (!scanTimedOut && (code != 0 || status != QProcess::NormalExit))
              emit message(
                  "ADB scan failed. Open Diagnostics for details, then retry.");
          });
  connect(&mirror, &QProcess::readyReadStandardOutput, this,
          &Engine::readMirrorOutput);
  connect(&mirror, &QProcess::readyReadStandardError, this, [this] {
    emit log(QString::fromUtf8(mirror.readAllStandardError()));
  });
  connect(&mirror, &QProcess::started, this, [this] {
    if (!stopping) {
      emit message(activeOtg
                       ? "Opening the local USB device. Waiting for AOA "
                         "control…"
                   : activeAudioOnly
                       ? "Connecting to the phone. Waiting for the first "
                         "audio packet…"
                       : "Connecting to the phone. Waiting for the first "
                         "video frame…");
      if (activeAudioOnly)
        audioCapabilityTimeout.start(2000);
      if (activeOtg)
        otgCapabilityTimeout.start(3000);
    }
    emit sessionChanged();
  });
  connect(&mirror, &QProcess::errorOccurred, this,
          [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
              stopTimeout.stop();
              audioCapabilityTimeout.stop();
              otgCapabilityTimeout.stop();
              bridgeCommandTimeout.stop();
              state = SessionState::Failed;
              stopping = false;
              const bool otg = activeOtg;
              activeSerial.clear();
              activeRecording.clear();
              activeTransportPinned = false;
              activeAdbServerSocket.clear();
              emit message(otg
                               ? "The bundled USB control engine could not "
                                 "start. See Diagnostics or reinstall "
                                 "DroidCast Desktop."
                               : "The bundled mirror engine could not start. "
                                 "See Diagnostics or reinstall DroidCast "
                                 "Desktop.");
              emit sessionChanged();
            }
          });
  connect(&mirror, &QProcess::finished, this,
          [this](int code, QProcess::ExitStatus status) {
            stopTimeout.stop();
            audioCapabilityTimeout.stop();
            otgCapabilityTimeout.stop();
            bridgeCommandTimeout.stop();
            pendingBridgeCommand = 0;
            readMirrorOutput();
            // Buffered acknowledgements cannot leave a completed session marked
            // as measuring after readMirrorOutput() processes its final lines.
            fpsMeasurementActive = false;
            const bool clean = code == 0 && status == QProcess::NormalExit &&
                               !forcedStop && !recordingFailed;
            const bool recording = !activeRecording.isEmpty();
            const bool otg = activeOtg;
            if (!clean && state != SessionState::Disconnected)
              state = SessionState::Failed;
            else if (state != SessionState::Disconnected &&
                     state != SessionState::Failed)
              state = SessionState::Ended;
            activeSerial.clear();
            activeRecording.clear();
            activeTransportPinned = false;
            activeAdbServerSocket.clear();
            if (forcedStop)
              emit message(otg ? "USB control was forced to stop. Check "
                                 "Diagnostics before reconnecting."
                               : "The engine was forced to stop. Any recording "
                                 "may be incomplete.");
            else if (recording && (!recordingFinalized || recordingFailed))
              emit message("Recording finalization was not confirmed. The file "
                           "may be incomplete; see Diagnostics.");
            else if (state == SessionState::Disconnected)
              emit message(otg ? "USB device disconnected. Reconnect it and "
                                 "start USB control again."
                               : "Phone disconnected. Reconnect it and start "
                                 "a new session.");
            else if (state == SessionState::Failed)
              emit message(otg ? (otgLifecycleFailure
                                      ? "USB control readiness was not "
                                        "confirmed. Reinstall this DroidCast "
                                        "build."
                                      : "USB control ended with an error. "
                                        "Check the cable, USB permissions and "
                                        "Diagnostics.")
                           : audioLifecycleFailure
                               ? "Audio-only is unavailable in the bundled "
                                 "engine. Reinstall DroidCast Desktop."
                               : "scrcpy exited with an error. Open "
                                 "Diagnostics for details.");
            else
              emit message(otg ? "USB control ended. You can reconnect or "
                                 "start another session."
                           : recording
                               ? "Recording finalized. Session ended."
                               : "Session ended. You can start another session.");
            stopping = false;
            emit fpsControlsChanged();
            emit appLaunchControlsChanged();
            emit sessionChanged();
          });
  wireless.setProcessChannelMode(QProcess::MergedChannels);
  connect(&wireless, &QProcess::stateChanged, this,
          [this] { emit wirelessChanged(); });
  connect(&wireless, &QProcess::started, this, [this] {
    if (!pairingInput.isEmpty()) {
      wireless.write(pairingInput);
      wireless.closeWriteChannel();
      pairingInput.fill('\0');
      pairingInput.clear();
    }
  });
  connect(&wireless, &QProcess::readyReadStandardOutput, this,
          [this] { wirelessOutput += wireless.readAllStandardOutput(); });
  connect(&wireless, &QProcess::errorOccurred, this,
          [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
              wirelessTimeout.stop();
              pairingInput.fill('\0');
              pairingInput.clear();
              emit message("The bundled connection service could not start. "
                           "See Diagnostics for details.");
            }
          });
  connect(&wireless, &QProcess::finished, this,
          [this](int code, QProcess::ExitStatus status) {
            wirelessTimeout.stop();
            wirelessOutput += wireless.readAllStandardOutput();
            auto result = QString::fromUtf8(wirelessOutput).trimmed();
            emit log(result);
            // ADB may return exit code zero even for a failed connect.
            bool success =
                code == 0 && status == QProcess::NormalExit &&
                (result.contains("connected to", Qt::CaseInsensitive) ||
                 result.contains("Successfully paired", Qt::CaseInsensitive));
            if (!wirelessTimedOut)
              emit message(success ? result
                                   : "Wireless request failed. " + result);
            if (success)
              refresh();
          });
  deviceTimeout.setSingleShot(true);
  connect(&deviceTimeout, &QTimer::timeout, this, [this] {
    deviceTimedOut = true;
    deviceTask.kill();
    const auto text =
        "The device operation timed out. Reconnect your phone and try again.";
    if (filePushTask)
      emit filePushMessage(text);
    emit message(text);
  });
  connect(&deviceTask, &QProcess::stateChanged, this,
          [this] { emit deviceTaskChanged(); });
  connect(&deviceTask, &QProcess::readyReadStandardOutput, this, [this] {
    deviceOutput += deviceTask.readAllStandardOutput();
    if (deviceOutput.size() > 32 * 1024 * 1024) {
      deviceTimedOut = true;
      deviceTask.kill();
      const auto text = filePushTask
                            ? "The transfer returned too much output and was "
                              "stopped."
                            : "The device returned too much data. The capture "
                              "was not saved.";
      if (filePushTask)
        emit filePushMessage(text);
      emit message(text);
    }
  });
  connect(&deviceTask, &QProcess::readyReadStandardError, this, [this] {
    const auto error = deviceTask.readAllStandardError();
    if (filePushTask)
      deviceOutput += error;
    emit log(QString::fromUtf8(error));
    if (filePushTask && deviceOutput.size() > 32 * 1024 * 1024) {
      deviceTimedOut = true;
      deviceTask.kill();
      const auto text =
          "The transfer returned too much output and was stopped.";
      emit filePushMessage(text);
      emit message(text);
    }
  });
  connect(
      &deviceTask, &QProcess::errorOccurred, this,
      [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
          deviceTimeout.stop();
          const auto text =
              "The bundled device service could not start. See Diagnostics.";
          if (filePushTask)
            emit filePushMessage(text);
          emit message(text);
        }
      });
  connect(
      &deviceTask, &QProcess::finished, this,
      [this](int code, QProcess::ExitStatus status) {
        deviceTimeout.stop();
        deviceOutput += deviceTask.readAllStandardOutput();
        if (deviceTimedOut || deviceAborted)
          return;
        if (code != 0 || status != QProcess::NormalExit) {
          if (filePushTask)
            emit filePushMessage(
                "File transfer failed. See Diagnostics for details.");
          emit message("Device operation failed. See Diagnostics for details.");
          return;
        }
        if (commandTask) {
          emit log(QString::fromUtf8(deviceOutput).trimmed());
          emit message("Phone command completed.");
        } else if (filePushTask) {
          const auto result = QString::fromUtf8(deviceOutput).trimmed();
          emit log(result);
          const bool failed = result.contains("error", Qt::CaseInsensitive) ||
                              result.contains("failed", Qt::CaseInsensitive) ||
                              !result.contains("pushed", Qt::CaseInsensitive);
          if (failed) {
            emit filePushMessage(
                "File transfer failed. See Diagnostics for details.");
            emit message("File transfer failed. See Diagnostics for details.");
          } else {
            emit filePushMessage("File copied to " + pushDestination);
            emit message("File copied to the Android destination.");
          }
        } else if (capturePath.isEmpty()) {
          const auto result = QString::fromUtf8(deviceOutput).trimmed();
          emit log(result);
          emit message(result.contains("Success")
                           ? "Application installed on your phone."
                           : "Installation failed. " + result);
        } else {
          if (QImage::fromData(deviceOutput, "PNG").isNull()) {
            emit message("The phone did not return a valid screenshot. "
                         "Nothing was saved.");
            return;
          }
          QSaveFile file(capturePath);
          if (!file.open(QIODevice::WriteOnly) ||
              file.write(deviceOutput) != deviceOutput.size() ||
              !file.commit()) {
            emit message("Could not save the screenshot. Check folder "
                         "permissions and available space.");
            return;
          }
          emit message("Screenshot saved to your capture library.");
          emit captureSaved(capturePath);
        }
      });
}

Engine::~Engine() {
  stopDeviceMonitoring();
  for (auto *process : {&scan, &wireless, &mirror, &deviceTask, &inspector,
                        &usbInspector, &deviceMonitor}) {
    if (process->state() != QProcess::NotRunning) {
      if (process == &mirror)
        process->write("Q", 1);
      else
        process->terminate();
      if (!process->waitForFinished(1500)) {
        process->kill();
        process->waitForFinished(1500);
      }
    }
  }
}

QProcessEnvironment Engine::processEnvironment(bool scrcpy) const {
  auto env = QProcessEnvironment::systemEnvironment();
  env.remove("ADB_SERVER_SOCKET");
  const auto socket = activeTransportPinned ? activeAdbServerSocket
                                            : adbServerSocket(preferences);
  if (!socket.isEmpty())
    env.insert("ADB_SERVER_SOCKET", socket);
  if (scrcpy) {
    env.insert("ADB", executablePath(preferences.adb));
    if (!preferences.server.isEmpty())
      env.insert("SCRCPY_SERVER_PATH", preferences.server);
  }
  return env;
}

void Engine::prepareAdbProcess(QProcess &process) const {
  process.setProcessEnvironment(processEnvironment(false));
}

void Engine::refresh() {
  if (monitorEnabled) { restartDeviceMonitor(); return; }
  if (scanning())
    return;
  if (otgSession()) {
    emit message("ADB discovery is paused during USB control.");
    return;
  }
  const auto issue = connectionIssue(preferences);
  if (!issue.isEmpty()) {
    devices.clear();
    emit devicesChanged();
    emit message(issue);
    return;
  }
  if (executablePath(preferences.adb).isEmpty()) {
    devices.clear();
    emit devicesChanged();
    emit message("DroidCast's bundled connection service is missing. Reinstall "
                 "the complete app package.");
    return;
  }
  scanOutput.clear();
  scanTimedOut = false;
  scanTimeout.start(10000);
  prepareAdbProcess(scan);
  scan.start(preferences.adb, {"devices", "-l"});
}

void Engine::startDeviceMonitoring() {
  if (monitorEnabled) return;
  monitorEnabled = true;
  monitorRetryDelay = 1000;
  if (scanning()) { scanTimeout.stop(); scan.kill(); }
  launchDeviceMonitor();
}

void Engine::stopDeviceMonitoring() {
  monitorEnabled = false; monitorRestart = false; monitorHealthy = false;
  monitorRecoveryPending = false;
  monitorRetry.stop(); monitorStartupTimeout.stop(); monitorOutput.clear();
  if (deviceMonitorRunning()) { deviceMonitor.terminate(); monitorStopTimeout.start(1500); }
}

void Engine::launchDeviceMonitor() {
  if (!monitorEnabled || deviceMonitorRunning() || otgSession()) return;
  monitorRetry.stop(); monitorOutput.clear();
  monitorHealthy = monitorFailed = monitorRestart = false;
  emit deviceMonitoringReset();
  devices.clear(); emit devicesChanged();
  const auto issue = connectionIssue(preferences);
  if (!issue.isEmpty() || executablePath(preferences.adb).isEmpty()) {
    failDeviceMonitor(issue.isEmpty() ? "The bundled ADB is missing. Device events are unavailable." : issue);
    return;
  }
  const auto env = processEnvironment(false);
  monitorSocket = env.value("ADB_SERVER_SOCKET");
  deviceMonitor.setProcessEnvironment(env);
  monitorStartupTimeout.start(10000);
  monitorUptime.start();
  // adb forwards the service's hex4-length framed stdout stream unchanged.
  deviceMonitor.start(preferences.adb, {"track-devices", "-l"});
}

void Engine::restartDeviceMonitor() {
  if (!monitorEnabled || monitorRestart) return;
  monitorRetry.stop(); monitorStartupTimeout.stop(); monitorHealthy = false;
  monitorRestart = true; monitorOutput.clear(); emit deviceMonitoringReset();
  devices.clear(); emit devicesChanged();
  if (deviceMonitorRunning()) { deviceMonitor.terminate(); monitorStopTimeout.start(1500); }
  else { monitorRestart = false; launchDeviceMonitor(); }
}

void Engine::scheduleMonitorRetry() {
  if (!monitorEnabled || otgSession() || monitorRetry.isActive()) return;
  monitorRetry.start(monitorRetryDelay);
  monitorRetryDelay = qMin(monitorRetryDelay * 2, 30000);
}

void Engine::failDeviceMonitor(const QString &reason) {
  if (!monitorEnabled || monitorFailed) return;
  if (monitorHealthy && monitorUptime.isValid() && monitorUptime.elapsed() >= 10000)
    monitorRetryDelay = 1000;
  monitorFailed = true; monitorHealthy = false; monitorStartupTimeout.stop(); monitorOutput.clear();
  monitorRecoveryPending = true;
  emit deviceMonitoringReset();
  devices.clear(); emit devicesChanged(); emit deviceScanCompleted(false); emit message(reason);
  if (deviceMonitorRunning()) deviceMonitor.kill();
  else scheduleMonitorRetry();
}

void Engine::readDeviceMonitor() {
  const auto output = deviceMonitor.readAllStandardOutput();
  if (!monitorEnabled || monitorFailed || monitorRestart) return;
  monitorOutput += output;
  if (monitorOutput.size() > 256 * 1024) { failDeviceMonitor("ADB device event output exceeded its limit."); return; }
  while (monitorOutput.size() >= 4) {
    const auto prefix = monitorOutput.left(4);
    for (char c : prefix)
      if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
        failDeviceMonitor("Invalid ADB device event framing."); return;
      }
    const auto length = prefix.toUInt(nullptr, 16);
    if (monitorOutput.size() < static_cast<qsizetype>(4 + length)) return;
    const auto payload = monitorOutput.mid(4, length);
    monitorOutput.remove(0, 4 + length);
    const bool recovered = !monitorHealthy && monitorRecoveryPending;
    if (!monitorHealthy) { monitorHealthy = true; monitorStartupTimeout.stop(); }
    devices = parseDevices(QString::fromUtf8(payload));
    emit devicesChanged(); emit deviceScanCompleted(true);
    if (recovered) { monitorRecoveryPending = false; emit message("Device discovery recovered."); }
  }
}

void Engine::inspectDevice(const QString &serial) {
  if (running() || inspecting()) {
    emit message("Finish the active session or inspection first.");
    return;
  }
  if (!canUseDevice(serial))
    return;
  inspectionSerial = serial;
  inspectionOutput.clear();
  inspectionAborted = false;
  emit inspectionResult(serial,
                        "Inspecting displays, encoders, cameras and apps…");
  auto env = processEnvironment(true);
  env.remove("DROIDCAST_SESSION_TOKEN");
  inspector.setProcessEnvironment(env);
  inspectionTimeout.start(20000);
  inspector.start(preferences.scrcpy,
                  {"--serial=" + serial, "--list-displays", "--list-encoders",
                   "--list-camera-sizes", "--list-apps"});
}

void Engine::cancelInspection() {
  if (!inspecting())
    return;
  inspectionAborted = true;
  inspectionTimeout.stop();
  inspector.kill();
  emit inspectionResult(
      inspectionSerial,
      "Inspection canceled. You can retry or start mirroring.");
}

void Engine::inspectOtgDevices() {
  if (running() || inspectingOtg()) {
    emit message("Finish the active session or USB scan first.");
    return;
  }
  if (executablePath(preferences.scrcpy).isEmpty()) {
    emit message("DroidCast's bundled USB engine is missing. Reinstall the "
                 "complete app package.");
    return;
  }
  otgInspectionOutput.clear();
  auto env = processEnvironment(true);
  env.remove("DROIDCAST_SESSION_TOKEN");
  usbInspector.setProcessEnvironment(env);
  otgInspectionTimeout.start(10000);
  usbInspector.start(preferences.scrcpy, {"--list-usb-devices"});
}

bool Engine::start(const QString &serial, const QString &recording) {
  if (running())
    return false;
  if (inspecting()) {
    emit message("Wait for device inspection to finish or cancel it before "
                 "starting a session.");
    return false;
  }
  const auto issue = sessionIssue(preferences, !recording.isEmpty());
  if (!issue.message.isEmpty()) {
    emit message(issue.message);
    return false;
  }
  const Device *selectedDevice = nullptr;
  for (const auto &device : devices)
    if (device.serial == serial && device.ready())
      selectedDevice = &device;
  if (!selectedDevice) {
    emit message("Select an authorized device before starting a session.");
    return false;
  }
  const bool acceptsInput =
      !preferences.options.value("audio-only", false).toBool() &&
      !preferences.options.value("no-control", false).toBool() &&
      preferences.options.value("video-source", "display") != "camera";
  const bool usesAoa =
      acceptsInput &&
      (preferences.keyboard == "aoa" || preferences.mouse == "aoa" ||
       preferences.options.value("gamepad") == "aoa");
  if (usesAoa &&
      (preferences.remoteAdb || selectedDevice->connection != "USB")) {
    emit message("AOA input requires a phone connected by local USB. Choose "
                 "SDK/UHID for wireless or remote ADB sessions.");
    return false;
  }
#ifdef Q_OS_WIN
  if (usesAoa) {
    emit message("AOA cannot share a mirrored USB phone with ADB on Windows. "
                 "Choose SDK/UHID; dedicated OTG mode is planned separately.");
    return false;
  }
#endif
  if (executablePath(preferences.scrcpy).isEmpty() ||
      executablePath(preferences.adb).isEmpty()) {
    emit message("DroidCast's engine bundle is incomplete. Reinstall the "
                 "complete app package.");
    return false;
  }
  if (!preferences.server.isEmpty() &&
      !QFileInfo(preferences.server).isFile()) {
    emit message("DroidCast's Android server is missing. Reinstall the "
                 "complete app package.");
    return false;
  }
  activeAdbServerSocket = adbServerSocket(preferences);
  activeTransportPinned = true;
  auto env = processEnvironment(true);
  activeSerial = serial;
  activeRecording = recording;
  activeOtg = false;
  activeAudioOnly = preferences.options.value("audio-only", false).toBool();
  activeAudioPlayback =
      !preferences.options.value("no-audio-playback", false).toBool();
  activeReadOnly = activeAudioOnly ||
                   preferences.options.value("no-control", false).toBool();
  activeCamera =
      !activeAudioOnly &&
      preferences.options.value("video-source", "display").toString() ==
          "camera";
  activeAlternateDisplay =
      !activeAudioOnly && !activeCamera &&
      (preferences.options.value("virtual-display", false).toBool() ||
       preferences.options.value("display-id", 0).toInt() != 0);
  activeFlexibleDisplay =
      !activeAudioOnly && !activeCamera &&
      preferences.options.value("virtual-display", false).toBool() &&
      preferences.options.value("flex-display", false).toBool() &&
      !activeReadOnly;
  stopping = false;
  state = SessionState::Starting;
  windowCommandsReady = false;
  windowCommandsHealthy = true;
  cameraCommandsReady = false;
  cameraCommandsHealthy = true;
  androidCommandsReady = false;
  androidCommandsHealthy = true;
  clipboardCommandsReady = false;
  clipboardCommandsHealthy = true;
  fpsCommandsReady = false;
  fpsCommandsHealthy = true;
  fpsMeasurementActive = false;
  appCommandsReady = false;
  appCommandsHealthy = true;
  audioLifecycleReady = false;
  audioLifecycleFailure = false;
  otgLifecycleFailure = false;
  pendingBridgeCommand = 0;
  emit windowControlMessage(activeAudioOnly
                                ? "Audio-only sessions have no mirror window."
                                : "Window controls become available after the "
                                  "first video frame.");
  emit cameraControlMessage(
      activeAudioOnly ? "Audio-only sessions have no camera controls."
      : activeCamera  ? "Camera controls become available after the "
                        "first video frame."
                      : "Start a camera session to use these controls.");
  emit androidControlMessage(
      activeAudioOnly ? "Audio-only sessions have no Android control channel."
      : activeCamera
          ? "Android display actions are unavailable during camera capture."
      : activeReadOnly
          ? "Android device actions are disabled in read-only mode."
          : "Android device actions become available after the first frame.");
  emit clipboardControlMessage(
      activeAudioOnly ? "Audio-only sessions have no clipboard controls."
      : activeCamera
          ? "Clipboard actions are unavailable during camera capture."
      : activeReadOnly
          ? "Clipboard actions are disabled in read-only mode."
          : "Clipboard actions become available after the first frame.");
  emit fpsControlMessage(activeAudioOnly
                             ? "Audio-only sessions do not render video frames."
                             : "Rendered FPS measurement becomes available "
                               "after the first frame.");
  emit appLaunchMessage(
      activeAudioOnly ? "App launch is unavailable in audio-only sessions."
      : activeCamera  ? "App launch is unavailable during camera capture."
      : activeReadOnly
          ? "App launch is disabled in read-only mode."
          : "App launch becomes available after the first display frame.");
  forcedStop = bridgeReady = recordingFinalized = recordingFailed = false;
  mirrorOutput.clear();
  sessionToken = QUuid::createUuid().toString(QUuid::Id128).toLatin1();
  env.insert("DROIDCAST_SESSION_TOKEN", QString::fromLatin1(sessionToken));
  mirror.setProcessEnvironment(env);
  const auto args = mirrorArguments(serial, preferences, recording);
  emit log("Starting " + preferences.scrcpy +
           "\nArguments: " + args.join(" | "));
  mirror.start(preferences.scrcpy, args);
  emit sessionChanged();
  return true;
}

bool Engine::startOtg(const QString &serial, bool keyboard, bool mouse,
                      bool gamepad) {
  if (running())
    return false;
  if (scanning() || wirelessBusy() || deviceBusy() || inspectingOtg()) {
    emit message("Wait for the current device or USB scan to finish before "
                 "starting USB control.");
    return false;
  }
  const auto selectedSerial = serial.trimmed();
  if (selectedSerial.size() > 256 ||
      selectedSerial.contains(QRegularExpression("[\\x00-\\x1f\\x7f]"))) {
    emit message("USB serial must be at most 256 characters and contain no "
                 "control characters.");
    return false;
  }
  if (!keyboard && !mouse && !gamepad) {
    emit message("Enable at least one USB control: keyboard, mouse or gamepad.");
    return false;
  }
  if (executablePath(preferences.scrcpy).isEmpty()) {
    emit message("DroidCast's control engine is missing. Reinstall the "
                 "complete app package.");
    return false;
  }

  activeSerial = selectedSerial.isEmpty() ? "Automatic USB selection"
                                          : selectedSerial;
  activeRecording.clear();
  activeOtg = true;
  activeAudioOnly = false;
  activeAudioPlayback = false;
  activeReadOnly = false;
  activeCamera = false;
  activeAlternateDisplay = false;
  activeFlexibleDisplay = false;
  activeTransportPinned = false;
  activeAdbServerSocket.clear();
  stopping = false;
  state = SessionState::Starting;
  windowCommandsReady = cameraCommandsReady = androidCommandsReady = false;
  clipboardCommandsReady = fpsCommandsReady = appCommandsReady = false;
  windowCommandsHealthy = cameraCommandsHealthy = androidCommandsHealthy = true;
  clipboardCommandsHealthy = fpsCommandsHealthy = appCommandsHealthy = true;
  fpsMeasurementActive = false;
  audioLifecycleReady = audioLifecycleFailure = false;
  otgLifecycleFailure = false;
  pendingBridgeCommand = 0;
  forcedStop = bridgeReady = recordingFinalized = recordingFailed = false;
  mirrorOutput.clear();

  emit windowControlMessage("USB control has no mirrored video window tools.");
  emit cameraControlMessage("USB control has no camera stream.");
  emit androidControlMessage(
      "Use the AOA keyboard, mouse or gamepad in the USB control window.");
  emit clipboardControlMessage("USB control has no Android clipboard channel.");
  emit fpsControlMessage("USB control does not render video frames.");
  emit appLaunchMessage("USB control has no ADB application channel.");

  sessionToken = QUuid::createUuid().toString(QUuid::Id128).toLatin1();
  auto env = processEnvironment(true);
  // OTG is a local libusb session. In particular, its optional Windows ADB
  // shutdown must never target a configured or inherited remote ADB server.
  env.remove("ADB_SERVER_SOCKET");
  env.insert("DROIDCAST_SESSION_TOKEN", QString::fromLatin1(sessionToken));
  mirror.setProcessEnvironment(env);
  QStringList args{"--otg",
                   keyboard ? "--keyboard=aoa" : "--keyboard=disabled",
                   mouse ? "--mouse=aoa" : "--mouse=disabled",
                   gamepad ? "--gamepad=aoa" : "--gamepad=disabled",
                   "--window-title=DroidCast USB control"};
  if (!selectedSerial.isEmpty())
    args.prepend("--serial=" + selectedSerial);
  emit log("Starting " + preferences.scrcpy +
           "\nArguments: " + args.join(" | "));
  mirror.start(preferences.scrcpy, args);
  emit sessionChanged();
  return true;
}

void Engine::stop() {
  if (!running() || stopping)
    return;
  stopping = true;
  audioCapabilityTimeout.stop();
  otgCapabilityTimeout.stop();
  bridgeCommandTimeout.stop();
  pendingBridgeCommand = 0;
  fpsMeasurementActive = false;
  emit fpsControlsChanged();
  emit appLaunchControlsChanged();
  state = SessionState::Stopping;
  emit message(activeOtg ? "Stopping USB control…"
                         : "Stopping session and finishing any recording…");
  // The fork handles this on its SDL event loop, following window-close
  // cleanup. QProcess buffers the byte if the process is still starting.
  mirror.write("Q", 1);
  stopTimeout.start(5000);
  emit sessionChanged();
}

QString Engine::sessionStateText() const {
  switch (state) {
  case SessionState::Idle:
    return "No active session";
  case SessionState::Starting:
    return activeOtg ? "Starting USB control" : "Starting session";
  case SessionState::Streaming:
    return activeOtg ? "Controlling over USB"
           : activeAudioOnly
               ? (activeAudioPlayback ? "Listening" : "Recording audio")
               : "Streaming";
  case SessionState::Stopping:
    return activeOtg ? "Stopping USB control" : "Finishing session";
  case SessionState::Ended:
    return "Session ended";
  case SessionState::Failed:
    return activeOtg ? "USB control failed" : "Session failed";
  case SessionState::Disconnected:
    return activeOtg ? "USB device disconnected" : "Phone disconnected";
  }
  return {};
}

void Engine::readMirrorOutput() {
  mirrorOutput += mirror.readAllStandardOutput();
  const QByteArray prefix = "DROIDCAST/1 " + sessionToken + ' ';
  qsizetype newline;
  while ((newline = mirrorOutput.indexOf('\n')) >= 0) {
    const auto line = mirrorOutput.left(newline).trimmed();
    mirrorOutput.remove(0, newline + 1);
    if (!line.startsWith(prefix)) {
      emit log(QString::fromUtf8(line));
      continue;
    }
    const auto event = line.mid(prefix.size());
    if (event == "bridge-ready")
      bridgeReady = true;
    else if (event == "otg-ready" && bridgeReady && activeOtg &&
             state == SessionState::Starting) {
      otgCapabilityTimeout.stop();
      state = SessionState::Streaming;
      emit message("USB control is ready. Focus the control window to send "
                   "keyboard, mouse or gamepad input.");
    } else if (event == "audio-lifecycle-ready" && bridgeReady) {
      audioLifecycleReady = true;
      audioCapabilityTimeout.stop();
    } else if (event == "first-audio-packet" && bridgeReady &&
               audioLifecycleReady && activeAudioOnly &&
               state == SessionState::Starting) {
      state = SessionState::Streaming;
      emit message(activeAudioPlayback
                       ? "Phone audio is playing on this computer."
                       : "Phone audio is being recorded without local "
                         "playback.");
    } else if (event == "first-frame" && bridgeReady && !activeAudioOnly &&
               !activeOtg &&
               state == SessionState::Starting) {
      state = SessionState::Streaming;
      emit message("Phone video is streaming in the mirror window.");
      emit windowControlMessage("Video is streaming. Window controls require "
                                "support from the bundled engine.");
    } else if (event == "window-controls-ready" && bridgeReady) {
      windowCommandsReady = true;
      emit windowControlMessage(activeFlexibleDisplay
                                    ? "Window controls ready. Resizing also "
                                      "changes the Android virtual display."
                                    : "Window controls ready. These actions "
                                      "affect the computer display only.");
    } else if (event == "fps-controls-ready" && bridgeReady) {
      fpsCommandsReady = true;
      emit fpsControlMessage(
          "Rendered FPS measurement ready. This is not latency or the "
          "configured capture limit.");
      emit fpsControlsChanged();
    } else if (event == "app-controls-ready" && bridgeReady && !activeCamera) {
      appCommandsReady = true;
      emit appLaunchMessage(
          activeReadOnly
              ? "App launch is disabled in read-only mode."
              : "App launch ready. Choose an application inspected for this "
                "phone.");
      emit appLaunchControlsChanged();
    } else if ((event == "fps-state:started" || event == "fps-state:stopped") &&
               bridgeReady) {
      fpsMeasurementActive = event.endsWith(":started");
      emit fpsControlMessage(fpsMeasurementActive
                                 ? "Measuring rendered FPS…"
                                 : "Rendered FPS measurement stopped.");
      emit fpsControlsChanged();
    } else if (event == "android-controls-ready" && bridgeReady &&
               !activeCamera) {
      androidCommandsReady = true;
      emit androidControlMessage(
          activeReadOnly
              ? "Android device actions are disabled in read-only mode."
              : "Android device actions ready. Requests use scrcpy's "
                "control channel.");
      emit androidControlsChanged();
    } else if (event == "camera-controls-ready" && bridgeReady &&
               activeCamera) {
      cameraCommandsReady = true;
      emit cameraControlMessage(
          activeReadOnly
              ? "Camera controls are disabled in read-only mode."
              : "Camera controls ready. Requests are sent to the phone; "
                "hardware support still varies.");
      emit cameraControlsChanged();
    } else if (event == "clipboard-controls-ready" && bridgeReady &&
               !activeCamera) {
      clipboardCommandsReady = true;
      emit clipboardControlMessage(
          activeReadOnly
              ? "Clipboard actions are disabled in read-only mode."
              : "Clipboard actions ready. Clipboard contents stay out of "
                "DroidCast diagnostics.");
      emit clipboardControlsChanged();
    } else if (event.startsWith("window-result:") && bridgeReady &&
               pendingBridgeCommand) {
      const QByteArray expected =
          QByteArray("window-result:") + pendingBridgeCommand + ':';
      if (event == expected + "handled" || event == expected + "unavailable") {
        bridgeCommandTimeout.stop();
        const bool handled = event.endsWith(":handled");
        const char command = pendingBridgeCommand;
        pendingBridgeCommand = 0;
        emit windowControlMessage(
            !handled ? "This action is unavailable. For resizing, leave "
                       "fullscreen or maximized mode first."
            : command == 'P'
                ? "Mirror image paused. Phone, audio and recording continue."
            : command == 'U' ? "Mirror image resumed."
                             : "Window request handled by the mirror. "
                               "Window-manager restrictions may still apply.");
        emit windowControlsChanged();
        emit cameraControlsChanged();
        emit androidControlsChanged();
        emit clipboardControlsChanged();
        emit fpsControlsChanged();
        emit appLaunchControlsChanged();
      }
    } else if (event.startsWith("camera-result:") && bridgeReady &&
               pendingBridgeCommand) {
      const QByteArray expected =
          QByteArray("camera-result:") + pendingBridgeCommand + ':';
      if (event == expected + "handled" || event == expected + "unavailable") {
        bridgeCommandTimeout.stop();
        const bool handled = event.endsWith(":handled");
        const char command = pendingBridgeCommand;
        pendingBridgeCommand = 0;
        emit cameraControlMessage(
            !handled
                ? "This camera action is unavailable. Resume a paused image "
                  "or check session control permissions."
            : command == 'T' ? "Torch-on request sent to the phone."
            : command == 't' ? "Torch-off request sent to the phone."
            : command == '+' ? "Zoom-in request sent to the phone."
                             : "Zoom-out request sent to the phone.");
        emit cameraControlsChanged();
        emit windowControlsChanged();
        emit androidControlsChanged();
        emit clipboardControlsChanged();
        emit fpsControlsChanged();
        emit appLaunchControlsChanged();
      }
    } else if (event.startsWith("android-result:") && bridgeReady &&
               pendingBridgeCommand) {
      const QByteArray expected =
          QByteArray("android-result:") + pendingBridgeCommand + ':';
      if (event == expected + "handled" || event == expected + "unavailable") {
        bridgeCommandTimeout.stop();
        const bool handled = event.endsWith(":handled");
        const char command = pendingBridgeCommand;
        pendingBridgeCommand = 0;
        QString handledMessage;
        switch (command) {
        case '0':
          handledMessage = "Android display-off request queued.";
          break;
        case '1':
          handledMessage = "Android display-on request queued.";
          break;
        case 'N':
          handledMessage = "Notification panel request queued.";
          break;
        case 'S':
          handledMessage = "Quick Settings request queued.";
          break;
        case 'C':
          handledMessage = "Collapse-panels request queued.";
          break;
        case 'D':
          handledMessage = "Android device rotation request queued.";
          break;
        case 'V':
          handledMessage = "Video reset request queued.";
          break;
        default:
          break;
        }
        emit androidControlMessage(
            handled ? handledMessage
                    : "This Android action is unavailable. Resume the image "
                      "or check the session mode and control permissions.");
        emit androidControlsChanged();
        emit windowControlsChanged();
        emit cameraControlsChanged();
        emit clipboardControlsChanged();
        emit fpsControlsChanged();
        emit appLaunchControlsChanged();
      }
    } else if (event.startsWith("clipboard-result:") && bridgeReady &&
               pendingBridgeCommand) {
      const QByteArray expected =
          QByteArray("clipboard-result:") + pendingBridgeCommand + ':';
      if (event == expected + "handled" || event == expected + "unavailable") {
        bridgeCommandTimeout.stop();
        const bool handled = event.endsWith(":handled");
        const char command = pendingBridgeCommand;
        pendingBridgeCommand = 0;
        emit clipboardControlMessage(
            !handled
                ? "This clipboard action is unavailable. Check the focused "
                  "Android app, clipboard content and session permissions."
            : command == 'Y'
                ? "Android selection copied to the computer clipboard."
                : "Computer clipboard paste request queued on Android.");
        emit clipboardControlsChanged();
        emit windowControlsChanged();
        emit cameraControlsChanged();
        emit androidControlsChanged();
        emit fpsControlsChanged();
        emit appLaunchControlsChanged();
      }
    } else if (event.startsWith("fps-result:") && bridgeReady &&
               pendingBridgeCommand) {
      const QByteArray expected =
          QByteArray("fps-result:") + pendingBridgeCommand + ':';
      if (event == expected + "handled" || event == expected + "unavailable") {
        bridgeCommandTimeout.stop();
        const bool handled = event.endsWith(":handled");
        const char command = pendingBridgeCommand;
        pendingBridgeCommand = 0;
        if (handled)
          fpsMeasurementActive = command == 'I';
        emit fpsControlMessage(
            !handled ? "FPS measurement is unavailable for this session."
            : command == 'I' ? "Measuring rendered FPS…"
                             : "Rendered FPS measurement stopped.");
        emit fpsControlsChanged();
        emit windowControlsChanged();
        emit cameraControlsChanged();
        emit androidControlsChanged();
        emit clipboardControlsChanged();
        emit appLaunchControlsChanged();
      }
    } else if (event.startsWith("fps-sample:") && bridgeReady &&
               fpsMeasurementActive) {
      static const QRegularExpression samplePattern(
          "\\Afps-sample:([0-9]{1,6}):([0-9]{1,9})\\z");
      const auto sample = samplePattern.match(QString::fromLatin1(event));
      if (sample.hasMatch()) {
        const auto rendered = sample.captured(1);
        const auto skipped = sample.captured(2).toUInt();
        emit fpsControlMessage(
            rendered + " rendered fps" +
            (skipped ? " · " + QString::number(skipped) + " skipped frame(s)"
                     : QString{}));
      }
    } else if (event.startsWith("app-result:") && bridgeReady &&
               pendingBridgeCommand == 'A') {
      if (event == "app-result:A:handled" ||
          event == "app-result:A:unavailable") {
        bridgeCommandTimeout.stop();
        const bool handled = event.endsWith(":handled");
        pendingBridgeCommand = 0;
        emit appLaunchMessage(
            handled ? "App launch request queued for the mirrored display."
                    : "This app launch is unavailable for the current "
                      "session.");
        emit appLaunchControlsChanged();
        emit windowControlsChanged();
        emit cameraControlsChanged();
        emit androidControlsChanged();
        emit clipboardControlsChanged();
        emit fpsControlsChanged();
      }
    } else if (event == "recording-finalized" && bridgeReady)
      recordingFinalized = true;
    else if (event == "recording-error" && bridgeReady)
      recordingFailed = true;
    else if (event == "disconnected" && bridgeReady)
      state = SessionState::Disconnected;
    else if (event == "failed" && bridgeReady)
      state = SessionState::Failed;
    emit sessionChanged();
  }
  if (mirrorOutput.size() > 65536) {
    emit log(QString::fromUtf8(mirrorOutput.left(65536)));
    mirrorOutput.clear();
  }
}

bool Engine::windowControlsAvailable() const {
  return running() && state == SessionState::Streaming && windowCommandsReady &&
         windowCommandsHealthy && !pendingBridgeCommand;
}

void Engine::windowAction(WindowAction action) {
  if (!windowControlsAvailable()) {
    emit windowControlMessage(
        "Window controls require a streaming session and no pending request.");
    return;
  }
  char command;
  switch (action) {
  case WindowAction::Fullscreen:
    command = 'F';
    break;
  case WindowAction::Fit:
    command = 'W';
    break;
  case WindowAction::PixelPerfect:
    command = 'Z';
    break;
  case WindowAction::RotateLeft:
    command = 'L';
    break;
  case WindowAction::RotateRight:
    command = 'R';
    break;
  case WindowAction::Pause:
    command = 'P';
    break;
  case WindowAction::Resume:
    command = 'U';
    break;
  default:
    return;
  }
  pendingBridgeCommand = command;
  if (mirror.write(&command, 1) != 1) {
    pendingBridgeCommand = 0;
    windowCommandsHealthy = false;
    emit windowControlMessage(
        "Could not send the window command. Restart the session.");
  } else {
    bridgeCommandTimeout.start(2000);
    emit windowControlMessage("Waiting for the mirror to handle the request…");
  }
  emit windowControlsChanged();
  emit cameraControlsChanged();
  emit androidControlsChanged();
  emit clipboardControlsChanged();
  emit fpsControlsChanged();
  emit appLaunchControlsChanged();
}

bool Engine::cameraControlsAvailable() const {
  return running() && state == SessionState::Streaming && activeCamera &&
         !activeReadOnly && cameraCommandsReady && cameraCommandsHealthy &&
         !pendingBridgeCommand;
}

void Engine::cameraAction(CameraAction action) {
  if (!cameraControlsAvailable()) {
    emit cameraControlMessage(
        !cameraSession() ? "Camera controls require an active camera session."
        : activeReadOnly
            ? "Camera controls are disabled in read-only mode."
            : "Camera controls are waiting for the stream or another request.");
    return;
  }
  char command;
  switch (action) {
  case CameraAction::TorchOn:
    command = 'T';
    break;
  case CameraAction::TorchOff:
    command = 't';
    break;
  case CameraAction::ZoomIn:
    command = '+';
    break;
  case CameraAction::ZoomOut:
    command = '-';
    break;
  default:
    return;
  }
  pendingBridgeCommand = command;
  if (mirror.write(&command, 1) != 1) {
    pendingBridgeCommand = 0;
    cameraCommandsHealthy = false;
    emit cameraControlMessage(
        "Could not send the camera request. Restart the session.");
  } else {
    bridgeCommandTimeout.start(2000);
    emit cameraControlMessage("Waiting for the camera to handle the request…");
  }
  emit cameraControlsChanged();
  emit windowControlsChanged();
  emit androidControlsChanged();
  emit clipboardControlsChanged();
  emit fpsControlsChanged();
  emit appLaunchControlsChanged();
}

bool Engine::androidControlsAvailable() const {
  return running() && state == SessionState::Streaming && !activeCamera &&
         !activeReadOnly && androidCommandsReady && androidCommandsHealthy &&
         !pendingBridgeCommand;
}

void Engine::androidAction(AndroidAction action) {
  if (!androidControlsAvailable()) {
    emit androidControlMessage(
        cameraSession()
            ? "Android display actions are unavailable during camera capture."
        : activeReadOnly
            ? "Android device actions are disabled in read-only mode."
            : "Android device actions are waiting for the stream or another "
              "request.");
    return;
  }
  char command;
  switch (action) {
  case AndroidAction::DisplayOff:
    command = '0';
    break;
  case AndroidAction::DisplayOn:
    command = '1';
    break;
  case AndroidAction::Notifications:
    command = 'N';
    break;
  case AndroidAction::QuickSettings:
    command = 'S';
    break;
  case AndroidAction::CollapsePanels:
    command = 'C';
    break;
  case AndroidAction::RotateDevice:
    command = 'D';
    break;
  case AndroidAction::ResetVideo:
    command = 'V';
    break;
  default:
    return;
  }
  pendingBridgeCommand = command;
  if (mirror.write(&command, 1) != 1) {
    pendingBridgeCommand = 0;
    androidCommandsHealthy = false;
    emit androidControlMessage(
        "Could not send the Android request. Restart the session.");
  } else {
    bridgeCommandTimeout.start(2000);
    emit androidControlMessage("Waiting for Android to queue the request…");
  }
  emit androidControlsChanged();
  emit windowControlsChanged();
  emit cameraControlsChanged();
  emit clipboardControlsChanged();
  emit fpsControlsChanged();
  emit appLaunchControlsChanged();
}

bool Engine::clipboardControlsAvailable() const {
  return running() && state == SessionState::Streaming && !activeCamera &&
         !activeReadOnly && clipboardCommandsReady &&
         clipboardCommandsHealthy && !pendingBridgeCommand;
}

void Engine::clipboardAction(ClipboardAction action) {
  if (!clipboardControlsAvailable()) {
    emit clipboardControlMessage(
        cameraSession()
            ? "Clipboard actions are unavailable during camera capture."
        : activeReadOnly ? "Clipboard actions are disabled in read-only mode."
                         : "Clipboard actions are waiting for the stream or "
                           "another request.");
    return;
  }
  const char command = action == ClipboardAction::CopyFromAndroid ? 'Y' : 'y';
  pendingBridgeCommand = command;
  if (mirror.write(&command, 1) != 1) {
    pendingBridgeCommand = 0;
    clipboardCommandsHealthy = false;
    emit clipboardControlMessage(
        "Could not send the clipboard request. Restart the session.");
  } else {
    bridgeCommandTimeout.start(2000);
    emit clipboardControlMessage(
        action == ClipboardAction::CopyFromAndroid
            ? "Waiting for Android to return the selected text…"
            : "Waiting for Android to accept the paste request…");
  }
  emit clipboardControlsChanged();
  emit windowControlsChanged();
  emit cameraControlsChanged();
  emit androidControlsChanged();
  emit fpsControlsChanged();
  emit appLaunchControlsChanged();
}

bool Engine::fpsControlsAvailable() const {
  return running() && state == SessionState::Streaming && fpsCommandsReady &&
         fpsCommandsHealthy && !pendingBridgeCommand;
}

void Engine::fpsAction(FpsAction action) {
  if (!fpsControlsAvailable()) {
    emit fpsControlMessage(
        "FPS measurement is waiting for video or another bridge request.");
    return;
  }
  const char command = action == FpsAction::Start ? 'I' : 'i';
  pendingBridgeCommand = command;
  if (mirror.write(&command, 1) != 1) {
    pendingBridgeCommand = 0;
    fpsCommandsHealthy = false;
    fpsMeasurementActive = false;
    emit fpsControlMessage(
        "Could not send the FPS request. Restart the session.");
  } else {
    bridgeCommandTimeout.start(2000);
    emit fpsControlMessage("Waiting for the mirror to update FPS measurement…");
  }
  emit fpsControlsChanged();
  emit windowControlsChanged();
  emit cameraControlsChanged();
  emit androidControlsChanged();
  emit clipboardControlsChanged();
  emit appLaunchControlsChanged();
}

bool Engine::appLaunchAvailable() const {
  return running() && state == SessionState::Streaming && !activeCamera &&
         !activeReadOnly && appCommandsReady && appCommandsHealthy &&
         !pendingBridgeCommand;
}

void Engine::launchApp(const QString &package) {
  const auto value = package.trimmed();
  if (value.isEmpty() || !textOptionError("start-app", value).isEmpty()) {
    emit appLaunchMessage(
        "Choose an inspected exact package; no launch request was sent.");
    return;
  }
  if (!appLaunchAvailable()) {
    emit appLaunchMessage(
        activeCamera     ? "App launch is unavailable during camera capture."
        : activeReadOnly ? "App launch is disabled in read-only mode."
                         : "App launch is waiting for the display stream or "
                           "another bridge request.");
    return;
  }

  const auto payload = value.toUtf8();
  QByteArray frame;
  frame.reserve(payload.size() + 2);
  frame.append('A');
  frame.append(static_cast<char>(payload.size()));
  frame.append(payload);
  pendingBridgeCommand = 'A';
  if (mirror.write(frame) != frame.size()) {
    pendingBridgeCommand = 0;
    appCommandsHealthy = false;
    emit appLaunchMessage(
        "Could not send the app-launch request. Restart the session.");
  } else {
    bridgeCommandTimeout.start(2000);
    emit appLaunchMessage(
        "Waiting for the mirror to queue the app-launch request…");
  }
  emit appLaunchControlsChanged();
  emit windowControlsChanged();
  emit cameraControlsChanged();
  emit androidControlsChanged();
  emit clipboardControlsChanged();
  emit fpsControlsChanged();
}

void Engine::connectWireless(const QString &endpoint, const QString &code) {
  if (otgSession()) {
    emit message("Wireless ADB is unavailable during USB control. Stop USB "
                 "control first.");
    return;
  }
  if (wirelessBusy())
    return;
  const auto transportIssue = connectionIssue(preferences);
  if (!transportIssue.isEmpty()) {
    emit message(transportIssue);
    return;
  }
  if (!validEndpoint(endpoint) ||
      (!code.isEmpty() &&
       !QRegularExpression("\\A[0-9]{6}\\z").match(code).hasMatch())) {
    emit message("Enter a host:port address and, for pairing, the six-digit "
                 "code shown on the phone.");
    return;
  }
  wirelessOutput.clear();
  wirelessTimedOut = false;
  pairingInput = code.isEmpty() ? QByteArray{} : code.toUtf8() + '\n';
  wirelessTimeout.start(30000);
  emit message(code.isEmpty() ? "Connecting to phone…" : "Pairing with phone…");
  prepareAdbProcess(wireless);
  wireless.start(preferences.adb,
                 {code.isEmpty() ? "connect" : "pair", endpoint});
}

bool Engine::canUseDevice(const QString &serial) {
  if (otgSession()) {
    emit message("ADB device tools are unavailable during USB control.");
    return false;
  }
  if (deviceBusy())
    return false;
  const auto issue = connectionIssue(preferences);
  if (!issue.isEmpty()) {
    emit message(issue);
    return false;
  }
  for (const auto &d : devices)
    if (d.serial == serial && d.ready())
      return true;
  emit message("Choose an authorized device first.");
  return false;
}

void Engine::capture(const QString &serial, const QString &path) {
  if (!captureAllowed(serial)) {
    emit message(
        audioOnlySession()
            ? "Screenshots are unavailable during an audio-only session."
        : cameraSession()
            ? "Screenshots do not capture the active camera stream. Use "
              "session recording instead; the phone display was not captured."
            : "Screenshots of active secondary or virtual displays are not "
              "supported yet. Use session recording instead; the primary "
              "display was not captured.");
    return;
  }
  if (!canUseDevice(serial) || path.isEmpty())
    return;
  capturePath = path;
  commandTask = false;
  filePushTask = false;
  deviceOutput.clear();
  deviceTimedOut = false;
  deviceAborted = false;
  deviceTimeout.start(20000);
  emit message("Capturing phone screen…");
  prepareAdbProcess(deviceTask);
  deviceTask.start(preferences.adb,
                   {"-s", serial, "exec-out", "screencap", "-p"});
}

void Engine::installApk(const QString &serial, const QString &path) {
  if (!controlAllowed()) {
    emit message(
        "APK installation is disabled while stopping or in read-only mode.");
    return;
  }
  if (running() && serial != activeSerial) {
    emit message("APK installation must target the active session.");
    return;
  }
  if (!canUseDevice(serial))
    return;
  if (!QFileInfo(path).isFile() ||
      !path.endsWith(".apk", Qt::CaseInsensitive)) {
    emit message("Choose an existing Android APK file.");
    return;
  }
  capturePath.clear();
  commandTask = false;
  filePushTask = false;
  deviceOutput.clear();
  deviceTimedOut = false;
  deviceAborted = false;
  deviceTimeout.start(120000);
  emit message("Installing application on your phone…");
  prepareAdbProcess(deviceTask);
  deviceTask.start(preferences.adb, {"-s", serial, "install", path});
}

void Engine::pushFile(const QString &serial, const QString &path,
                      const QString &destination) {
  if (running() && serial != activeSerial) {
    const auto text = "File transfer must target the active session phone.";
    emit filePushMessage(text);
    emit message(text);
    return;
  }
  if (deviceBusy()) {
    emit filePushMessage(
        "Another device operation is in progress. Wait for it to finish.");
    return;
  }
  if (!canUseDevice(serial)) {
    emit filePushMessage("Choose an authorized device first.");
    return;
  }

  const QFileInfo source(path);
  if (!source.isFile() || !source.isReadable()) {
    const auto text = "Choose an existing readable regular file.";
    emit filePushMessage(text);
    emit message(text);
    return;
  }
  const auto rawDestination = destination.trimmed();
  const auto cleanDestination = QDir::cleanPath(rawDestination);
  const bool unsafeComponent =
      rawDestination.split('/', Qt::KeepEmptyParts).contains(".") ||
      rawDestination.split('/', Qt::KeepEmptyParts).contains("..");
  const bool invalidDestination =
      rawDestination.size() > 1024 || !rawDestination.startsWith('/') ||
      cleanDestination == "/" || unsafeComponent ||
      QRegularExpression("[\\x00-\\x1f\\x7f]").match(rawDestination).hasMatch();
  if (invalidDestination) {
    const auto text =
        "Use an absolute Android directory without . or .. components.";
    emit filePushMessage(text);
    emit message(text);
    return;
  }

  const auto targetDirectory = cleanDestination + '/';
  capturePath.clear();
  commandTask = false;
  filePushTask = true;
  pushDestination = targetDirectory + source.fileName();
  deviceOutput.clear();
  deviceTimedOut = false;
  deviceAborted = false;
  deviceTimeout.start(300000);
  emit filePushMessage("Sending " + source.fileName() + " to " +
                       targetDirectory + "…");
  emit message("Sending file to the Android device…");
  prepareAdbProcess(deviceTask);
  deviceTask.start(
      preferences.adb,
      {"-s", serial, "push", source.absoluteFilePath(), targetDirectory});
}

void Engine::cancelDeviceTask() {
  if (deviceTask.state() == QProcess::NotRunning)
    return;
  deviceAborted = true;
  deviceTimeout.stop();
  deviceTask.kill();
  if (filePushTask)
    emit filePushMessage("File transfer canceled.");
  emit message("Device operation canceled.");
}

bool Engine::controlAllowed() const {
  return !stopping &&
         !(running() ? (activeReadOnly || activeOtg)
                     : preferences.options.value("no-control", false).toBool());
}

void Engine::phoneAction(const QString &serial, PhoneAction action) {
  if (cameraSession()) {
    emit message("Phone-toolbar commands are disabled during camera capture. "
                 "No command was sent to the Android display.");
    return;
  }
  if (usesAlternateDisplay()) {
    emit message("Phone-toolbar commands are disabled for secondary or virtual "
                 "displays. Use input inside the mirror window; no command was "
                 "sent to the primary display.");
    return;
  }
  if (!controlAllowed()) {
    emit message(
        "Phone controls are disabled while stopping or in read-only mode.");
    return;
  }
  if (running() && serial != activeSerial) {
    emit message("Phone controls must target the active session.");
    return;
  }
  if (!canUseDevice(serial))
    return;
  QString key;
  switch (action) {
  case PhoneAction::Back:
    key = "KEYCODE_BACK";
    break;
  case PhoneAction::Home:
    key = "KEYCODE_HOME";
    break;
  case PhoneAction::Recents:
    key = "KEYCODE_APP_SWITCH";
    break;
  case PhoneAction::Power:
    key = "KEYCODE_POWER";
    break;
  case PhoneAction::VolumeUp:
    key = "KEYCODE_VOLUME_UP";
    break;
  case PhoneAction::VolumeDown:
    key = "KEYCODE_VOLUME_DOWN";
    break;
  default:
    return;
  }
  commandTask = true;
  filePushTask = false;
  capturePath.clear();
  deviceOutput.clear();
  deviceTimedOut = false;
  deviceAborted = false;
  deviceTimeout.start(10000);
  prepareAdbProcess(deviceTask);
  deviceTask.start(preferences.adb,
                   {"-s", serial, "shell", "input", "keyevent", key});
}
