/**
 * @file MidiToSerial.ino
 * @author Phil Schatzmann
 * @brief Midi output over Serial 

 * @copyright Copyright (c) 2021
 * 
 */
#include "Midi.h"

BluetoothSerial SerialBT;
MidiStreamOut out(SerialBT);
uint16_t note = 64; // 0 to 128
uint16_t amplitude = 100; // 0 to 128

void setup() {
    Serial.begin(115200);
    SerialBT.begin("Bluetooth-Midi");
    
    Serial.println("starting loop...");
}

void loop() {
    Serial.println();
    Serial.print("playing ");
    Serial.println(++note);

    out.noteOn( note, amplitude );
    delay(900);
    out.noteOff( note, 20 );
    delay(200);
    if (note>=90) {
      note = 30;
    }
}
