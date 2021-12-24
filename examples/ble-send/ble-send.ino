/**
 * @file MidiBLEServer.ino
 * @author Phil Schatzmann
 * @brief Example BLE Midi which sends out midi messages
 * @version 0.1
 * @date 2021-12-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <Midi.h>

MidiBleServer ble("MidiServer");

void setup() {
  Serial.begin(115200);
  ble.begin(action);
  ble.setDefaultSendingChannel(0);
}

void loop() {
  Serial.print("playing ");
  Serial.println(++note);

  ble.noteOn( note, amplitude );
  delay(900);
  ble.noteOff( note, 20 );
  delay(200);
  if (note>=90) {
    note = 30;
  }
}


