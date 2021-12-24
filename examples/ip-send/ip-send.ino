/**
 * @file ip-send.ino
 * @author Phil Schatzmann
 * @brief Midi output over ip 

 * @copyright Copyright (c) 2021
 * 
 */
#include "Midi.h"

#include <WiFi.h>
#include <WiFiMulti.h>

IPAddress target_ip(192, 168, 1, 38);
int target_port = 5008;
WiFiClient client;
const char *SSID = "your ssid";
const char *PWD = "your password";
MidiStreamOut out(client);

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
  client.connect(target_ip, target_port);
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
