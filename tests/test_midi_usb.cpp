// Tests for the USB MIDI support: packet conversion, parser, MidiUsbBase and
// the Linux raw MIDI implementation (tested with a FIFO as loopback device)
#include <stdlib.h>
#include <sys/stat.h>
#include <deque>
#include <vector>

#include "MidiLogger.h"
#include "MidiUsb.h"
#include "TestHelper.h"

using namespace midi;

typedef std::vector<uint8_t> Bytes;

static Bytes packetOf(uint8_t cable, Bytes msg) {
  uint8_t packet[4];
  if (!MidiUsbPacket::fromMessage(cable, msg.data(), (int)msg.size(), packet))
    return {};
  return Bytes(packet, packet + 4);
}

static void testFromMessage() {
  CHECK(packetOf(0, {0x90, 60, 100}) == Bytes({0x09, 0x90, 60, 100}));
  CHECK(packetOf(0, {0x83, 60, 0}) == Bytes({0x08, 0x83, 60, 0}));
  CHECK(packetOf(1, {0xB2, 7, 99}) == Bytes({0x1B, 0xB2, 7, 99}));
  CHECK(packetOf(0, {0xC5, 12}) == Bytes({0x0C, 0xC5, 12, 0}));
  CHECK(packetOf(0, {0xD0, 77}) == Bytes({0x0D, 0xD0, 77, 0}));
  CHECK(packetOf(0, {0xE0, 0, 64}) == Bytes({0x0E, 0xE0, 0, 64}));
  CHECK(packetOf(0, {0xA0, 60, 20}) == Bytes({0x0A, 0xA0, 60, 20}));
  // system common / realtime
  CHECK(packetOf(0, {0xF8}) == Bytes({0x0F, 0xF8, 0, 0}));
  CHECK(packetOf(0, {0xF6}) == Bytes({0x05, 0xF6, 0, 0}));
  CHECK(packetOf(0, {0xF3, 5}) == Bytes({0x02, 0xF3, 5, 0}));
  CHECK(packetOf(0, {0xF2, 1, 2}) == Bytes({0x03, 0xF2, 1, 2}));
  // cable is limited to 0..15, data to 7 bits
  CHECK(packetOf(0x1F, {0x90, 0xFF, 0x80}) == Bytes({0xF9, 0x90, 0x7F, 0x00}));
  // unsupported or incomplete
  CHECK(packetOf(0, {0xF0, 1, 0xF7}).empty());
  CHECK(packetOf(0, {0x40, 1, 2}).empty());
  CHECK(packetOf(0, {0x90, 60}).empty());
  CHECK(packetOf(0, {}).empty());
}

static void testToBytes() {
  uint8_t bytes[3];
  uint8_t note[4] = {0x29, 0x91, 60, 100};
  CHECK_EQ(MidiUsbPacket::toBytes(note, bytes), 3);
  CHECK_EQ(bytes[0], 0x91);
  CHECK_EQ(MidiUsbPacket::cable(note), 2);
  CHECK_EQ(MidiUsbPacket::cin(note), 9);
  uint8_t pc[4] = {0x0C, 0xC0, 5, 0};
  CHECK_EQ(MidiUsbPacket::toBytes(pc, bytes), 2);
  uint8_t rt[4] = {0x0F, 0xF8, 0, 0};
  CHECK_EQ(MidiUsbPacket::toBytes(rt, bytes), 1);
  uint8_t reserved[4] = {0x01, 0x90, 1, 2};
  CHECK_EQ(MidiUsbPacket::toBytes(reserved, bytes), 0);
}

static void testParseUsb() {
  RecordingAction14 action;
  MidiParser parser(&action);
  uint8_t packets[] = {
      0x09, 0x90, 60, 100,   // note on
      0x19, 0x91, 61, 0,     // note on velocity 0 = note off, cable 1
      0x0F, 0xF8, 0,  0,     // clock: ignored
      0x04, 0xF0, 1,  2,     // sysex start: ignored
      0x07, 3,    4,  0xF7,  // sysex end with 3 bytes
      0x0E, 0xE0, 0,  64,    // pitch bend center
      0x0C, 0xC3, 9,  0,     // program change
      0x0B, 0xB0, 7,  99,    // control change
      0x00, 0x90, 1,  1,     // reserved CIN: ignored
      0x09, 0x90};           // incomplete: ignored
  parser.parseUsb(packets, sizeof(packets));
  CHECK(action.take() == Events({"on 0 60 100", "off 1 61 0", "pb14 0 8192",
                                 "pc 3 9", "cc 0 7 99"}));
}

static void testParseUsbChannelFilter() {
  RecordingAction action;
  MidiParser parser(&action, 1);
  uint8_t packets[] = {0x09, 0x90, 60, 100, 0x09, 0x91, 62, 100};
  parser.parseUsb(packets, sizeof(packets));
  CHECK(action.take() == Events({"on 1 62 100"}));
}

/// MidiUsbBase with a memory based transport
class MockUsb : public MidiUsbBase {
 public:
  using MidiUsbBase::MidiUsbBase;
  std::deque<Bytes> input;
  std::vector<Bytes> output;

  bool writePacket(const uint8_t packet[4]) override {
    output.push_back(Bytes(packet, packet + 4));
    return true;
  }
  bool readPacket(uint8_t packet[4]) override {
    if (input.empty()) return false;
    for (int j = 0; j < 4; j++) packet[j] = input.front()[j];
    input.pop_front();
    return true;
  }
};

static void testBaseSend() {
  MockUsb usb;
  usb.noteOn(60, 100);
  usb.noteOff(60, 0, 3);
  usb.programChange(7);
  usb.channelPressure(50, 2);
  usb.pitchBend(8192);
  usb.setCable(2);
  usb.controlChange(7, 99);
  CHECK(usb.output == std::vector<Bytes>({{0x09, 0x90, 60, 100},
                                          {0x08, 0x83, 60, 0},
                                          {0x0C, 0xC0, 7, 0},
                                          {0x0D, 0xD2, 50, 0},
                                          {0x0E, 0xE0, 0, 64},
                                          {0x2B, 0xB0, 7, 99}}));
  uint8_t clock = 0xF8;
  CHECK(usb.writeMessage(&clock, 1));
  CHECK(usb.output.back() == Bytes({0x2F, 0xF8, 0, 0}));
}

static void testBaseReceive() {
  RecordingAction action;
  MockUsb usb(&action);
  CHECK(!usb.loop());
  usb.input.push_back({0x09, 0x90, 60, 100});
  usb.input.push_back({0x08, 0x80, 60, 0});
  CHECK(usb.loop());
  CHECK(action.take() == Events({"on 0 60 100", "off 0 60 0"}));

  // channel filter set later
  usb.setFilterReceivingChannel(1);
  usb.input.push_back({0x09, 0x90, 60, 100});
  usb.input.push_back({0x09, 0x91, 61, 100});
  usb.loop();
  CHECK(action.take() == Events({"on 1 61 100"}));
}

static void testBaseWithoutAction() {
  // packets are consumed even without an action
  MockUsb usb;
  usb.input.push_back({0x09, 0x90, 60, 100});
  CHECK(usb.loop());
  CHECK(usb.input.empty());
}

static void testBaseCustomParser() {
  RecordingAction action;
  MidiParser parser(&action, 2);
  MockUsb usb;
  usb.setMidiParser(parser);
  usb.input.push_back({0x09, 0x92, 60, 100});
  usb.input.push_back({0x09, 0x90, 60, 100});
  usb.loop();
  CHECK(action.take() == Events({"on 2 60 100"}));
}

static void testDeviceAndHostOnDesktop() {
  // on the Linux desktop both use the ALSA raw MIDI implementation
  CHECK(MIDI_USB_DEVICE_RAWMIDI);
  CHECK(MIDI_USB_HOST_RAWMIDI);
  MidiUsbHost host;
  CHECK(!host.isConnected());
  CHECK(!host.begin("/nonexistent/midi"));
}

static void testRawMidiLoopback() {
  // A FIFO opened for read and write returns what was written
  char dir[] = "/tmp/midi-usb-test-XXXXXX";
  CHECK(mkdtemp(dir) != nullptr);
  std::string fifo = std::string(dir) + "/midi";
  CHECK(mkfifo(fifo.c_str(), 0600) == 0);

  RecordingAction action;
  MidiUsbDevice device(nullptr, &action);
  CHECK(device.begin(fifo.c_str()));
  CHECK(device.isConnected());
  CHECK(!device.loop());

  device.noteOn(60, 100, 1);
  device.controlChange(7, 99);
  device.programChange(3);
  CHECK(device.loop());
  CHECK(action.take() == Events({"on 1 60 100", "cc 0 7 99", "pc 0 3"}));

  // raw bytes with running status
  uint8_t raw[] = {0x92, 60, 100, 62, 100};
  CHECK(device.writeBytes(raw, sizeof(raw)));
  device.loop();
  CHECK(action.take() == Events({"on 2 60 100", "on 2 62 100"}));

  device.end();
  CHECK(!device.writeBytes(raw, sizeof(raw)));
  unlink(fifo.c_str());
  rmdir(dir);
}

static void testRawMidiCardName() {
  CHECK(MidiUsbRawMidi::cardName("/dev/snd/invalid") == "");
  // just make sure that listing works on any system
  for (auto& dev : MidiUsbRawMidi::devices()) {
    CHECK(dev.rfind("/dev/snd/midiC", 0) == 0);
  }
}

int main() {
  MidiLogLevel = MidiError;
  testFromMessage();
  testToBytes();
  testParseUsb();
  testParseUsbChannelFilter();
  testBaseSend();
  testBaseReceive();
  testBaseWithoutAction();
  testBaseCustomParser();
  {
    StdoutCapture capture;  // suppress the expected error log
    testDeviceAndHostOnDesktop();
  }
  testRawMidiLoopback();
  testRawMidiCardName();
  return testResult("test_midi_usb");
}
