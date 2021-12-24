/**
 * @file MidiBLEClient.ino
 * @author Phil Schatzmann
 * @brief Example BLE Midi which receives midi messages
 * Midi messages and process them by generating sound.
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <Midi.h>


MidiCallbackAction action;
MidiBleServer ble("MidiServer", &action);

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("onNoteOn: ");
  Serial.println(note);
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("onNoteOff: ");
  Serial.println(note);
}

void setup() {
  Serial.begin(115200);

  action.setCallbacks(onNoteOn, onNoteOff);
  ble.begin(action);
}

void loop() {
}
