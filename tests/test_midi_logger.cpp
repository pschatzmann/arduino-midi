#include <string.h>
#include "MidiLogger.h"
#include "TestHelper.h"

static void testLevelFilter() {
  MidiLogLevel = MidiWarning;
  StdoutCapture cap;
  MIDI_LOGD("debug %d", 1);
  MIDI_LOGI("info %d", 2);
  MIDI_LOGW("warning %d", 3);
  MIDI_LOGE("error %d", 4);
  std::string out = cap.stop();
  CHECK(out.find("debug 1") == std::string::npos);
  CHECK(out.find("info 2") == std::string::npos);
  CHECK(out.find("Warning") != std::string::npos);
  CHECK(out.find("warning 3") != std::string::npos);
  CHECK(out.find("Error") != std::string::npos);
  CHECK(out.find("error 4") != std::string::npos);
}

static void testDebugLevelLogsEverything() {
  MidiLogLevel = MidiDebug;
  StdoutCapture cap;
  MIDI_LOGD("debug");
  MIDI_LOGI("info");
  std::string out = cap.stop();
  CHECK(out.find("Debug") != std::string::npos);
  CHECK(out.find("Info") != std::string::npos);
}

static void testFormatWithoutArguments() {
  MidiLogLevel = MidiInfo;
  StdoutCapture cap;
  MIDI_LOGI("plain message");
  std::string out = cap.stop();
  CHECK(out.find("plain message") != std::string::npos);
}

static void testLongMessageIsTruncated() {
  // must not overflow the internal buffer
  MidiLogLevel = MidiInfo;
  std::string longText(1000, 'x');
  StdoutCapture cap;
  MIDI_LOGI("%s", longText.c_str());
  std::string out = cap.stop();
  CHECK(out.size() > 100);
  CHECK(out.size() < 250);
  CHECK(out.find("xxxx") != std::string::npos);
}

int main() {
  testLevelFilter();
  testDebugLevelLogsEverything();
  testFormatWithoutArguments();
  testLongMessageIsTruncated();
  MidiLogLevel = MidiWarning;
  return testResult("test_midi_logger");
}
