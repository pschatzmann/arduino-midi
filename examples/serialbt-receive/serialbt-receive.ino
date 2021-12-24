/**
 * @file serialbt-receive.ino
 * @author Phil Schatzmann
 * @brief Receive Midi Messages over Bluetooth - see https://www.pschatzmann.ch/home/2021/12/22/esp32-audiokit-and-midi-using-the-bluetooth-serial-interface/
 * @date 2021-12-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "Midi.h"
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
MidiCallbackAction action;
MidiStreamIn in(SerialBT, action);

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
  action.setCallbacks(onNoteOn, onNoteOff);
  SerialBT.begin("Bluetooth-Midi");

  Serial.println("starting loop...");
}

void loop() {
  if (!in.loop()){
    delay(10);
  }
}


