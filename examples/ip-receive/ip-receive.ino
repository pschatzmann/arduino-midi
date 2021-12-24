/**
 * @file ip-receive.ino
 * @author Phil Schatzmann
 * @brief Midi input over ip 

 * @copyright Copyright (c) 2021
 * 
 */
#include "Midi.h"

#include <WiFi.h>
#include <WiFiMulti.h>

IPAddress target_ip(192, 168, 1, 38);
int target_port = 5008;
WiFiClient client;
MidiCallbackAction action;
MidiStreamIn in(client, action);

const char *SSID = "your ssid";
const char *PWD = "your password";

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("onNoteOn: ");
  Serial.println(note);
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("onNoteOff: ");
  Serial.println(note);
}

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

  // setup command handler
  action.setCallbacks(onNoteOn, onNoteOff);

  // setup ip
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
