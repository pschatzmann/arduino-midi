/**
 * @file MidiToIP.ino
 * @author Phil Schatzmann
 * @brief Example of handling midi input over ip - sound beneration requires arduino-stk
 * 
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <WiFi.h>
#include <WiFiMulti.h>
#include "ArdMidiStreamOut.h"

using namespace midi;
using namespace stk;

IPAddress ip(192, 168, 1, 35);
int port = 9999;
WiFiClient client;
ArdMidiStreamOut out(client);
const char *SSID = "your ssid";
const char *PWD = "your password";

uint16_t note = 64; // 0 to 128
uint16_t amplitude = 100; // 0 to 128

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  Serial.print("Connected to IP address: ");
  Serial.println(WiFi.localIP());

  client.connect(ip, port);
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
