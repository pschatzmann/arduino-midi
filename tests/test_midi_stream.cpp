// Tests for MidiStreamIn / MidiStreamOut: needs the Arduino Emulator
#include "MidiStreamIn.h"
#include "MidiStreamOut.h"
#include "MidiLogger.h"
#include "TestHelper.h"

using namespace midi;

typedef std::vector<uint8_t> Bytes;

/// Memory based Stream: input is provided by the test, output is recorded
class BufferStream : public Stream {
 public:
  Bytes input;
  Bytes output;
  size_t pos = 0;

  void add(const Bytes& data) { input.insert(input.end(), data.begin(), data.end()); }

  int available() override { return input.size() - pos; }
  int read() override { return pos < input.size() ? input[pos++] : -1; }
  int peek() override { return pos < input.size() ? input[pos] : -1; }
  size_t write(uint8_t b) override {
    output.push_back(b);
    return 1;
  }
  size_t write(const uint8_t* data, size_t len) override {
    output.insert(output.end(), data, data + len);
    return len;
  }
};

static void testStreamInParses() {
  BufferStream stream;
  RecordingAction action;
  MidiStreamIn in(stream, action);
  CHECK(!in.loop());  // no data
  stream.add({0x90, 60, 100, 0xB0, 7, 99});
  CHECK(in.loop());
  CHECK(action.take() == Events({"on 0 60 100", "cc 0 7 99"}));
  CHECK_EQ(stream.available(), 0);
}

static void testStreamInMoreThanBuffer() {
  BufferStream stream;
  RecordingAction action;
  MidiStreamIn in(stream, action);
  Events expected;
  stream.add({0x90});
  for (int j = 0; j < 100; j++) {
    stream.add({(uint8_t)j, 100});  // running status
    expected.push_back("on 0 " + std::to_string(j) + " 100");
  }
  CHECK(in.loop());
  CHECK(action.take() == expected);
}

static void testStreamInSplitOverLoops() {
  BufferStream stream;
  RecordingAction action;
  MidiStreamIn in(stream, action);
  stream.add({0x90, 60});
  in.loop();
  CHECK(action.take().empty());
  stream.add({100, 61});
  in.loop();
  CHECK(action.take() == Events({"on 0 60 100"}));
  stream.add({90});
  in.loop();
  CHECK(action.take() == Events({"on 0 61 90"}));
}

static void testStreamInWithCustomParser() {
  BufferStream stream;
  RecordingAction action;
  MidiParser parser(&action, 1);  // not owned by MidiStreamIn
  {
    MidiStreamIn in(stream, parser);
    stream.add({0x90, 60, 100, 0x91, 61, 100});
    in.loop();
  }
  CHECK(action.take() == Events({"on 1 61 100"}));
  // the parser is still usable after MidiStreamIn was destroyed
  uint8_t msg[] = {0x91, 62, 1};
  parser.parse(msg, sizeof(msg));
  CHECK(action.take() == Events({"on 1 62 1"}));
}

static void testStreamOut() {
  BufferStream stream;
  MidiStreamOut out(stream);
  out.noteOn(60, 100, 1);
  out.noteOn(61, 100, 1);  // status is repeated
  out.programChange(5, 1);
  out.pitchBend(8192, 1);
  CHECK(stream.output ==
        Bytes({0x91, 60, 100, 0x91, 61, 100, 0xC1, 5, 0xE1, 0x00, 0x40}));
}

static void testStreamOutWithoutStream() {
  MidiStreamOut out;
  out.noteOn(60, 100);  // must not crash
  CHECK(true);
}

static void testRoundTrip() {
  BufferStream wire;
  MidiStreamOut out(wire);
  out.noteOn(60, 100, 9);
  out.polyPressure(60, 20, 9);
  out.channelPressure(30, 9);
  out.noteOff(60, 0, 9);
  wire.add(wire.output);
  RecordingAction action;
  MidiStreamIn in(wire, action);
  in.loop();
  CHECK(action.take() ==
        Events({"on 9 60 100", "pp 9 60 20", "cp 9 30", "off 9 60 0"}));
}

int main() {
  MidiLogLevel = MidiWarning;
  testStreamInParses();
  testStreamInMoreThanBuffer();
  testStreamInSplitOverLoops();
  testStreamInWithCustomParser();
  testStreamOut();
  testStreamOutWithoutStream();
  testRoundTrip();
  return testResult("test_midi_stream");
}
