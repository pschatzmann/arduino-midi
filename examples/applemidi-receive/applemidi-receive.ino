/**
 * @file applemidi-receive.ino
 * @author Phil Schatzmann
 * @brief Receiving Midi messages using Apple Midi
 * @date 2021-12-24
 * 
 * @copyright Copyright (c) 2021
 */

#include <WiFi.h>
#include "Midi.h"

MidiCallbackAction action;
AppleMidiServer apple(&action);

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
  Serial.begin(119200);
  MidiLogLevel = MidiInfo;

  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  Serial.print("Connected to IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  action.setCallbacks(onNoteOn, onNoteOff);
  setupWIFI();
  apple.begin(5004);
}

void loop() {
  if (!apple.loop()){
    delay(10);
  }
}


