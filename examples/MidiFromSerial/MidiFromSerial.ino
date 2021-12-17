/**
 * @file MidiFromSerial.ino
 * @author Phil Schatzmann
 * @brief Example of handling midi input over a serial stream - sound beneration requires arduino-stk
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <Midi.h>
#include <StkAll.h>

ArdI2SOut i2s;
Clarinet clarinet(440);
StkMidiAction action(&clarinet);
MidiEventHandler handler(&action);
MidiStreamIn in(Serial, handler);

void setup() {
  Serial.begin(115200);
}

void loop() {
  in.loop();
  i2s.tick( handler.tick() );
}
