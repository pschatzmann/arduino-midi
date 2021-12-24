#pragma once

#define MIDI_ACTIVE true

#if defined(ESP32) || defined(ESP8266)
#  define MIDI_BLE_ACTIVE true
#  define APPLE_MIDI_ACTIVE true
#  define MDNS_ACTIVE true
#else
#  define APPLE_MIDI_ACTIVE false
#  define MIDI_BLE_ACTIVE false
#  define MDNS_ACTIVE false
#endif

