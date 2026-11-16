#include "MidiCallbackAction.h"
#include "MidiParser.h"
#include "TestHelper.h"

using namespace midi;

static Events events;

static void testCallbacks() {
  MidiCallbackAction action;
  action.setCallbacks(
      [](uint8_t ch, uint8_t n, uint8_t v) { events.push_back("on " + std::to_string(n)); },
      [](uint8_t ch, uint8_t n, uint8_t v) { events.push_back("off " + std::to_string(n)); },
      [](uint8_t ch, uint8_t c, uint8_t v) { events.push_back("cc " + std::to_string(c)); },
      [](uint8_t ch, uint8_t v) { events.push_back("pb " + std::to_string(v)); });
  action.setCallbackOnProgramChange(
      [](uint8_t ch, uint8_t p) { events.push_back("pc " + std::to_string(p)); });
  action.setCallbackOnChannelPressure(
      [](uint8_t ch, uint8_t p) { events.push_back("cp " + std::to_string(p)); });
  action.setCallbackOnPolyPressure(
      [](uint8_t ch, uint8_t n, uint8_t p) { events.push_back("pp " + std::to_string(n)); });

  MidiParser parser(&action);
  uint8_t msg[] = {0x90, 60, 100, 0x80, 60, 0,  0xB0, 64, 127, 0xE0, 0, 0x40,
                   0xC0, 3,       0xD0, 9,      0xA0, 61, 2};
  parser.parse(msg, sizeof(msg));
  CHECK(events == Events({"on 60", "off 60", "cc 64", "pb 64", "pc 3", "cp 9", "pp 61"}));
  events.clear();
}

static void testPitchBend14Callback() {
  MidiCallbackAction action;
  action.setCallbackOnPitchBend14(
      [](uint8_t ch, uint16_t v) { events.push_back("pb14 " + std::to_string(v)); });
  MidiParser parser(&action);
  uint8_t msg[] = {0xE0, 0x7F, 0x7F};
  parser.parse(msg, sizeof(msg));
  CHECK(events == Events({"pb14 16383"}));
  events.clear();
}

static void testNoCallbacks() {
  MidiCallbackAction action;
  MidiParser parser(&action);
  uint8_t msg[] = {0x90, 60, 100, 0xE0, 0, 0x40, 0xC0, 3};
  parser.parse(msg, sizeof(msg));  // must not crash
  CHECK(events.empty());
}

static void testPitchBend14HasPriority() {
  // when the 14 bit callback is defined, the 7 bit one is not called
  MidiCallbackAction action;
  action.setCallbackOnPitchBend(
      [](uint8_t ch, uint8_t v) { events.push_back("pb " + std::to_string(v)); });
  action.setCallbackOnPitchBend14(
      [](uint8_t ch, uint16_t v) { events.push_back("pb14 " + std::to_string(v)); });
  MidiParser parser(&action);
  uint8_t msg[] = {0xE0, 0x00, 0x40};
  parser.parse(msg, sizeof(msg));
  CHECK(events == Events({"pb14 8192"}));
  events.clear();
}

static void testIndividualSetters() {
  MidiCallbackAction action;
  action.setCallbackOnNoteOn([](uint8_t ch, uint8_t n, uint8_t v) {
    events.push_back("on " + std::to_string(ch) + " " + std::to_string(n) +
                     " " + std::to_string(v));
  });
  action.setCallbackOnNoteOff([](uint8_t ch, uint8_t n, uint8_t v) {
    events.push_back("off " + std::to_string(ch) + " " + std::to_string(n) +
                     " " + std::to_string(v));
  });
  action.setCallbackOnControlChange([](uint8_t ch, uint8_t c, uint8_t v) {
    events.push_back("cc " + std::to_string(ch) + " " + std::to_string(c) +
                     " " + std::to_string(v));
  });
  action.setCallbackOnPitchBend([](uint8_t ch, uint8_t v) {
    events.push_back("pb " + std::to_string(ch) + " " + std::to_string(v));
  });
  MidiParser parser(&action);
  uint8_t msg[] = {0x92, 60, 100, 0x82, 60, 7, 0xB2, 1, 2, 0xE2, 0x7F, 0x7F};
  parser.parse(msg, sizeof(msg));
  CHECK(events ==
        Events({"on 2 60 100", "off 2 60 7", "cc 2 1 2", "pb 2 127"}));
  events.clear();
}

static void testUnsetCallback() {
  MidiCallbackAction action;
  action.setCallbackOnNoteOn(
      [](uint8_t ch, uint8_t n, uint8_t v) { events.push_back("on"); });
  action.setCallbackOnNoteOn(nullptr);
  MidiParser parser(&action);
  uint8_t msg[] = {0x90, 60, 100};
  parser.parse(msg, sizeof(msg));
  CHECK(events.empty());
}

static void testDeleteViaBasePointer() {
  // MidiAction has a virtual destructor
  MidiAction* action = new MidiCallbackAction();
  delete action;
  CHECK(true);
}

int main() {
  testCallbacks();
  testPitchBend14Callback();
  testNoCallbacks();
  testPitchBend14HasPriority();
  testIndividualSetters();
  testUnsetCallback();
  testDeleteViaBasePointer();
  return testResult("test_midi_callback_action");
}
