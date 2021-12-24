/**
 * @file serial-receive.ino
 * @author Phil Schatzmann
 * @brief Receive MIDI messages from Serial port - see https://www.pschatzmann.ch/home/2021/12/22/esp32-audiokit-and-midi-using-serial/
 * @date 2021-12-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "Midi.h"

#define RXD2 21
#define TXD2 22

MidiCallbackAction action;
MidiStreamIn in(Serial, action);

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("onNoteOn: ");
  Serial.println(note);
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("onNoteOff: ");
  Serial.println(note);
}

void setup() {
  Serial.begin(119200);
  Serial2.begin(119200, SERIAL_8N1, RXD2, TXD2);
  action.setCallbacks(onNoteOn, onNoteOff);
}

void loop() {
  if (!in.loop()){
    delay(10);
  }
}
