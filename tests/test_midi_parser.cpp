#include "MidiParser.h"
#include "MidiLogger.h"
#include "TestHelper.h"

using namespace midi;

static void testNoteOnOff() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0x91, 60, 100, 0x81, 60, 64};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 1 60 100", "off 1 60 64"}));
}

static void testNoteOnVelocityZeroIsNoteOff() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0x90, 60, 0};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"off 0 60 0"}));
}

static void testRunningStatus() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0x90, 60, 100, 62, 90, 64, 0};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 0 60 100", "on 0 62 90", "off 0 64 0"}));
}

static void testRunningStatusOverMultipleCalls() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg1[] = {0xB0, 7, 100};
  uint8_t msg2[] = {10, 64};
  parser.parse(msg1, sizeof(msg1));
  parser.parse(msg2, sizeof(msg2));
  CHECK(action.take() == Events({"cc 0 7 100", "cc 0 10 64"}));
}

static void testSplitMessage() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0x90, 60, 100};
  for (uint8_t b : msg) parser.parse(&b, 1);
  CHECK(action.take() == Events({"on 0 60 100"}));
}

static void testOneDataByteMessages() {
  RecordingAction action;
  MidiParser parser(&action);
  // program change with running status, then channel pressure
  uint8_t msg[] = {0xC3, 5, 6, 0xD2, 40, 41};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"pc 3 5", "pc 3 6", "cp 2 40", "cp 2 41"}));
}

static void testPolyPressure() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0xA0, 60, 33};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"pp 0 60 33"}));
}

static void testPitchBend7Bit() {
  // default MidiAction implementation forwards the MSB
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0xE0, 0x00, 0x40, 0xE0, 0x7F, 0x7F, 0xE0, 0x00, 0x00};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"pb 0 64", "pb 0 127", "pb 0 0"}));
}

static void testPitchBend14Bit() {
  RecordingAction14 action;
  MidiParser parser(&action);
  // LSB first, then MSB
  uint8_t msg[] = {0xE5, 0x00, 0x40, 0xE5, 0x7F, 0x7F, 0xE5, 0x01, 0x00};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"pb14 5 8192", "pb14 5 16383", "pb14 5 1"}));
}

static void testRealtimeDoesNotBreakMessage() {
  RecordingAction action;
  MidiParser parser(&action);
  // timing clock in the middle of a note on
  uint8_t msg[] = {0x90, 60, 0xF8, 100, 0xFE, 62, 90};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 0 60 100", "on 0 62 90"}));
}

static void testSysexIsIgnored() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0x90, 60, 100, 0xF0, 0x7E, 0x00, 0x09, 0x01, 0xF7,
                   61, 1,  // no running status after sysex
                   0x90, 62, 90};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 0 60 100", "on 0 62 90"}));
}

static void testDataWithoutStatusIsIgnored() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {60, 100, 0x90, 60, 100};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 0 60 100"}));
}

static void testChannelFilter() {
  RecordingAction action;
  MidiParser parser(&action, 1);
  uint8_t msg[] = {0x90, 60, 100, 0x91, 61, 100, 0xB2, 7, 1};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 1 61 100"}));
}

static void testWithoutAction() {
  MidiParser parser;
  uint8_t msg[] = {0x90, 60, 100};
  parser.parse(msg, sizeof(msg));  // must not crash
  CHECK(true);
}

static void testSystemCommonClearsRunningStatus() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0x90, 60, 100,
                   0xF3, 5,       // song select
                   61, 1,         // no running status
                   0x90, 62, 90,
                   0xF2, 1, 2,    // song position
                   63, 1,
                   0x90, 64, 80,
                   0xF1, 3,       // MTC quarter frame
                   0xF6,          // tune request
                   65, 1};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() ==
        Events({"on 0 60 100", "on 0 62 90", "on 0 64 80"}));
}

static void testStatusInterruptsIncompleteMessage() {
  RecordingAction action;
  MidiParser parser(&action);
  // the incomplete note on is discarded
  uint8_t msg[] = {0x90, 60, 0xB0, 7, 100};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"cc 0 7 100"}));
}

static void testStatusTerminatesSysex() {
  RecordingAction action;
  MidiParser parser(&action);
  // sysex without 0xF7: the next status ends it
  uint8_t msg[] = {0xF0, 1, 2, 0x90, 60, 100};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 0 60 100"}));
}

static void testRealtimeInsideSysex() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0xF0, 1, 0xF8, 2, 0xF7, 0x90, 60, 100};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 0 60 100"}));
}

static void testRealtimeKeepsRunningStatus() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0x90, 60, 100, 0xFA, 0xFC, 0xFF, 61, 90};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 0 60 100", "on 0 61 90"}));
}

static void testReset() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg1[] = {0x90, 60};
  parser.parse(msg1, sizeof(msg1));
  parser.reset();
  // 100 must not complete the note on
  uint8_t msg2[] = {100, 0x90, 61, 1};
  parser.parse(msg2, sizeof(msg2));
  CHECK(action.take() == Events({"on 0 61 1"}));
}

static void testBeginResetsState() {
  RecordingAction action1;
  RecordingAction action2;
  MidiParser parser(&action1);
  uint8_t msg1[] = {0x90, 60};
  parser.parse(msg1, sizeof(msg1));
  parser.begin(&action2);
  uint8_t msg2[] = {100, 0x91, 61, 1};
  parser.parse(msg2, sizeof(msg2));
  CHECK(action1.take().empty());
  CHECK(action2.take() == Events({"on 1 61 1"}));
}

static void testChannelFilterWithRunningStatus() {
  RecordingAction action;
  MidiParser parser(&action, 3);
  uint8_t msg[] = {0x93, 60, 100, 61, 100, 0x92, 62, 100, 63, 100};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 3 60 100", "on 3 61 100"}));
}

static void testAllChannels() {
  RecordingAction action;
  MidiParser parser(&action);
  Events expected;
  for (int ch = 0; ch < 16; ch++) {
    uint8_t msg[] = {(uint8_t)(0x90 | ch), 60, 100};
    parser.parse(msg, sizeof(msg));
    expected.push_back("on " + std::to_string(ch) + " 60 100");
  }
  CHECK(action.take() == expected);
}

static void testEmptyInput() {
  RecordingAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0x90};
  parser.parse(msg, 0);
  parser.parse(nullptr, 0);
  parser.parse(msg, -1);
  CHECK(action.take().empty());
}

static void testDebugLogOfLargeInput() {
  // the hex dump must not overflow the stack or the log buffer
  RecordingAction action;
  MidiParser parser(&action);
  std::vector<uint8_t> msg;
  for (int j = 0; j < 2000; j++) {
    msg.push_back(0x90);
    msg.push_back(60);
    msg.push_back(100);
  }
  MidiLogLevel = MidiDebug;
  {
    StdoutCapture cap;
    parser.parse(msg.data(), msg.size());
    std::string out = cap.stop();
    CHECK(out.find("parse: len: 6000") != std::string::npos);
  }
  MidiLogLevel = MidiWarning;
  CHECK_EQ(action.take().size(), 2000u);
}

/// Parser subclass which intercepts events
class CountingParser : public MidiParser {
 public:
  using MidiParser::MidiParser;
  int noteOnCount = 0;
  void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) override {
    noteOnCount++;
  }
};

static void testSubclassOverride() {
  RecordingAction action;
  CountingParser parser(&action);
  uint8_t msg[] = {0x90, 60, 100, 61, 100, 0x80, 60, 0};
  parser.parse(msg, sizeof(msg));
  CHECK_EQ(parser.noteOnCount, 2);
  CHECK(action.take() == Events({"off 0 60 0"}));
}

int main() {
  testNoteOnOff();
  testNoteOnVelocityZeroIsNoteOff();
  testRunningStatus();
  testRunningStatusOverMultipleCalls();
  testSplitMessage();
  testOneDataByteMessages();
  testPolyPressure();
  testPitchBend7Bit();
  testPitchBend14Bit();
  testRealtimeDoesNotBreakMessage();
  testSysexIsIgnored();
  testDataWithoutStatusIsIgnored();
  testChannelFilter();
  testWithoutAction();
  testSystemCommonClearsRunningStatus();
  testStatusInterruptsIncompleteMessage();
  testStatusTerminatesSysex();
  testRealtimeInsideSysex();
  testRealtimeKeepsRunningStatus();
  testReset();
  testBeginResetsState();
  testChannelFilterWithRunningStatus();
  testAllChannels();
  testEmptyInput();
  testDebugLogOfLargeInput();
  testSubclassOverride();
  return testResult("test_midi_parser");
}
