#include "MidiCommon.h"
#include "MidiParser.h"
#include "MidiLogger.h"
#include "TestHelper.h"

using namespace midi;

/// Captures the generated messages as raw midi bytes
class CaptureOut : public MidiCommon {
 public:
  std::vector<uint8_t> bytes;
  int writeCount = 0;

  // expose protected functionality for testing
  using MidiCommon::updateTimestamp;
  using MidiCommon::setConnectionStatus;
  MidiAction* action() { return pMidiAction; }

 protected:
  void writeData(MidiMessage* msg, int len) override {
    writeCount++;
    bytes.push_back(msg->status);
    if (len >= 1) bytes.push_back(msg->arg1);
    if (len >= 2) bytes.push_back(msg->arg2);
  }
};

typedef std::vector<uint8_t> Bytes;

static void testNoteOnOff() {
  CaptureOut out;
  out.noteOn(60, 100, 2);
  out.noteOff(60, 0, 2);
  CHECK(out.bytes == Bytes({0x92, 60, 100, 0x82, 60, 0}));
}

static void testDefaultSendingChannel() {
  CaptureOut out;
  out.setDefaultSendingChannel(3);
  out.noteOn(60, 100);
  out.controlChange(7, 99);
  CHECK(out.bytes == Bytes({0x93, 60, 100, 0xB3, 7, 99}));
}

static void testPitchBend() {
  CaptureOut out;
  out.pitchBend(8192, 0);
  out.pitchBend(16383, 0);
  out.pitchBend(1, 0);
  CHECK(out.bytes ==
        Bytes({0xE0, 0x00, 0x40, 0xE0, 0x7F, 0x7F, 0xE0, 0x01, 0x00}));
}

static void testOneByteMessages() {
  CaptureOut out;
  out.programChange(5, 1);
  out.channelPressure(40, 1);
  CHECK(out.bytes == Bytes({0xC1, 5, 0xD1, 40}));
}

static void testPolyPressure() {
  CaptureOut out;
  out.polyPressure(60, 33, 1);
  CHECK(out.bytes == Bytes({0xA1, 60, 33}));
}

static void testChannelModeMessages() {
  CaptureOut out;
  out.allNotesOff(0);
  out.resetAllControllers(0);
  out.localControl(true, 0);
  CHECK(out.bytes == Bytes({0xB0, 120, 0, 0xB0, 121, 0, 0xB0, 122, 127}));
}

static void testRoundTrip() {
  // everything we generate must be parsed back to the same values
  CaptureOut out;
  out.noteOn(64, 90, 9);
  out.pitchBend(12345, 9);
  out.programChange(42, 9);
  out.polyPressure(64, 10, 9);
  RecordingAction14 action;
  MidiParser parser(&action);
  parser.parse(out.bytes.data(), out.bytes.size());
  CHECK(action.take() ==
        Events({"on 9 64 90", "pb14 9 12345", "pc 9 42", "pp 9 64 10"}));
}

static void testNoteFrequency() {
  CHECK(MidiCommon::noteToFrequency(69) > 439.99f);
  CHECK(MidiCommon::noteToFrequency(69) < 440.01f);
  CHECK(MidiCommon::noteToFrequency(60) > 261.62f);
  CHECK(MidiCommon::noteToFrequency(60) < 261.63f);
  CHECK(MidiCommon::noteToFrequency(0) > 8.17f);
  CHECK(MidiCommon::noteToFrequency(0) < 8.18f);
  CHECK_EQ(MidiCommon::frequencyToNote(440.0f), 69);
  CHECK_EQ(MidiCommon::frequencyToNote(880.0f), 81);
}

static void testFrequencyToNoteRoundTrip() {
  // every note must survive the conversion: no truncation errors
  for (int note = 0; note < 128; note++) {
    float freq = MidiCommon::noteToFrequency(note);
    CHECK_EQ(MidiCommon::frequencyToNote(freq), note);
  }
  // nearest note
  CHECK_EQ(MidiCommon::frequencyToNote(445.0f), 69);
  CHECK_EQ(MidiCommon::frequencyToNote(460.0f), 70);
}

static void testFrequencyToNoteRange() {
  CHECK_EQ(MidiCommon::frequencyToNote(1.0f), 0);
  CHECK_EQ(MidiCommon::frequencyToNote(0.0f), 0);
  CHECK_EQ(MidiCommon::frequencyToNote(-5.0f), 0);
  CHECK_EQ(MidiCommon::frequencyToNote(20000.0f), 127);
}

static void testExplicitChannelOverridesDefault() {
  CaptureOut out;
  out.setDefaultSendingChannel(3);
  out.noteOn(60, 100, 0);
  out.noteOn(60, 100, 15);
  out.noteOn(60, 100);
  CHECK(out.bytes == Bytes({0x90, 60, 100, 0x9F, 60, 100, 0x93, 60, 100}));
}

static void testInvalidChannelIsMasked() {
  // must not abort and must not corrupt the status nibble
  CaptureOut out;
  out.noteOn(60, 100, 17);
  out.setDefaultSendingChannel(18);
  out.noteOn(60, 100);
  CHECK(out.bytes == Bytes({0x91, 60, 100, 0x92, 60, 100}));
}

static void testDataBytesAreMasked() {
  // data bytes must never have the high bit set
  CaptureOut out;
  out.noteOn(200, 255, 0);
  out.noteOff(200, 255, 0);
  out.controlChange(200, 255, 0);
  out.programChange(200, 0);
  out.channelPressure(200, 0);
  out.polyPressure(200, 255, 0);
  for (size_t j = 0; j < out.bytes.size(); j++) {
    bool isStatus = j == 0 || j == 3 || j == 6 || j == 9 || j == 11 || j == 13;
    CHECK_EQ((out.bytes[j] & 0x80) != 0, isStatus);
  }
}

static void testPitchBendRange() {
  // values above 16383 are clamped instead of wrapping around
  CaptureOut out;
  out.pitchBend(0, 0);
  out.pitchBend(16384, 0);
  out.pitchBend(65535, 0);
  CHECK(out.bytes == Bytes({0xE0, 0x00, 0x00, 0xE0, 0x7F, 0x7F, 0xE0, 0x7F, 0x7F}));
}

static void testControlChange() {
  CaptureOut out;
  out.controlChange(MODULATION, 64, 4);
  CHECK(out.bytes == Bytes({0xB4, 1, 64}));
}

static void testDeprecatedPolyPressureSendsNothing() {
  CaptureOut out;
  MidiLogLevel = MidiError;
  out.polyPressure(33);
  MidiLogLevel = MidiWarning;
  CHECK(out.bytes.empty());
}

static void testWritePassesThrough() {
  CaptureOut out;
  MidiMessage msg{};
  msg.status = 0x95;
  msg.arg1 = 1;
  msg.arg2 = 2;
  out.write(&msg, 2);
  CHECK(out.bytes == Bytes({0x95, 1, 2}));
  CHECK_EQ(out.writeCount, 1);
}

static void testBaseWriteDataDoesNothing() {
  // the default implementation must be callable without output
  MidiCommon common;
  common.noteOn(60, 100);
  CHECK(true);
}

static void testUpdateTimestamp() {
  CaptureOut out;
  MidiMessage msg{};
  out.updateTimestamp(&msg);
  CHECK_EQ(msg.timestampHigh, 0x80);
  CHECK_EQ(msg.timestampLow, 0x81);
  // wrap of the low 7 bits increments the high part
  for (int j = 0; j < 127; j++) out.updateTimestamp(&msg);
  CHECK_EQ(msg.timestampHigh, 0x81);
  CHECK_EQ(msg.timestampLow, 0x80);
  // full wrap around: header must always be 0b10xxxxxx
  for (int j = 0; j < 128 * 64; j++) {
    out.updateTimestamp(&msg);
    CHECK_EQ(msg.timestampHigh & 0xC0, 0x80);
    CHECK_EQ(msg.timestampLow & 0x80, 0x80);
  }
}

static void testMidiActionAndConnectionStatus() {
  CaptureOut out;
  CHECK(out.action() == nullptr);
  CHECK_EQ(out.getConnectionStatus(), Unconnected);
  RecordingAction action;
  out.setMidiAction(action);
  CHECK(out.action() == &action);
  out.setConnectionStatus(Connected);
  CHECK_EQ(out.getConnectionStatus(), Connected);
}

static void testRoundTripAllMessages() {
  CaptureOut out;
  out.noteOn(60, 1, 15);
  out.noteOff(60, 2, 15);
  out.controlChange(7, 3, 15);
  out.programChange(4, 15);
  out.channelPressure(5, 15);
  out.polyPressure(60, 6, 15);
  out.pitchBend(0, 15);
  out.allNotesOff(15);
  RecordingAction14 action;
  MidiParser parser(&action);
  parser.parse(out.bytes.data(), out.bytes.size());
  CHECK(action.take() == Events({"on 15 60 1", "off 15 60 2", "cc 15 7 3",
                                 "pc 15 4", "cp 15 5", "pp 15 60 6",
                                 "pb14 15 0", "cc 15 120 0"}));
}

int main() {
  testNoteOnOff();
  testDefaultSendingChannel();
  testPitchBend();
  testOneByteMessages();
  testPolyPressure();
  testChannelModeMessages();
  testRoundTrip();
  testNoteFrequency();
  testFrequencyToNoteRoundTrip();
  testFrequencyToNoteRange();
  testExplicitChannelOverridesDefault();
  testInvalidChannelIsMasked();
  testDataBytesAreMasked();
  testPitchBendRange();
  testControlChange();
  testDeprecatedPolyPressureSendsNothing();
  testWritePassesThrough();
  testBaseWriteDataDoesNothing();
  testUpdateTimestamp();
  testMidiActionAndConnectionStatus();
  testRoundTripAllMessages();
  return testResult("test_midi_common");
}
