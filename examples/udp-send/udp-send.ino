/**
 * @file MidiToUDP.ino
 * @author Phil Schatzmann
 * @brief Example for output of midi message over ip
 * @version 0.1
 * @date 2021-12-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "WiFi.h" 
#include "MidiUdp.h" 
#include "MidiStreamOut.h"

using namespace midi;

IPAddress ip(192, 168, 1, 35);
MidiUdp udp(ip, 7000);
MidiStreamOut out(udp);
const char *SSID = "your ssid";
const char *PWD = "your password";

uint16_t note = 64; // 0 to 128
uint16_t amplitude = 100; // 0 to 128

void setupWIFI() {
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  Serial.print("Connected to IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  setupWIFI();    
}

void loop() {
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
