/**
 * @file MidiFromSerial.ino
 * @author Phil Schatzmann
 * @brief Example of handling midi input over a serial stream - sound beneration requires arduino-stk
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "Voicer.h"
#include "Clarinet.h"
#include "ArdMidiStreamIn.h"

using namespace midi;
using namespace stk;

StkMidiVoicer voicer;
Clarinet clarinet(440);
ArdMidiEventHandler handler(&voicer);
ArdMidiStreamIn in(Serial, handler);

void setup() {
  Serial.begin(115200);
  
  voicer.addInstrument(&clarinet);
}

void loop() {
  in.loop();
}
