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
#include <StkAll.h>

Clarinet clarinet(440);
StkMidiAction action; // or implement your own handler
MidiBleServer ble("MidiServer", &action);

AudioKitStream kit; // audio output to ESP32 AudioKit
ArdStreamOut output(kit);

void setup() {
  Serial.begin(115200);

  action.addInstrument(&clarinet, 0);
  ble.start(action);
}

void loop() {
  output.tick( action.tick() );
}
