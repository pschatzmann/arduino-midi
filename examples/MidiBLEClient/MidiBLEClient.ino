/**
 * @file MidiBLEClient.ino
 * @author Phil Schatzmann
 * @brief Example BLE Midi Client - requires arduino-stk to generate sound
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <Clarinet.h>
#include <ArdMidiVoicer.h>
#include <MidiBleEventHandler.h>
#include <MidiBleServer.h>

using namespace midi;
using namespace stk;

MidiVoicer voicer;
Clarinet clarinet(440);
MidiBleEventHandler handler(&voicer);
MidiBleServer ble("MidiServer", &handler);

void setup() {
  Serial.begin(115200);

  voicer.addInstrument(&clarinet, 0);
  ble.start(voicer);
}

uint16_t note = 64; // 0 to 128
uint16_t amplitude = 100; // 0 to 128

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
