/**
 * @file MidiFromSerial.ino
 * @author Phil Schatzmann
 * @brief Example of handling midi input over a serial stream - sound beneration requires arduino-stk
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "Clarinet.h"
#include <ArdMidiVoicer.h>
#include "MidiStreamIn.h"

using namespace midi;
using namespace stk;

MidiVoicer voicer;
Clarinet clarinet(440);
MidiEventHandler handler(&voicer);
MidiStreamIn in(Serial, handler);

void setup() {
  Serial.begin(115200);
  
  voicer.addInstrument(&clarinet);
}

void loop() {
  in.loop();
}
