#pragma once

// Arduino is optional: the core (parser, message generation) also builds
// on a desktop without Arduino (e.g. for the unit tests)
#if defined(ARDUINO)
#  define MIDI_ARDUINO_ACTIVE 1
#elif defined(__has_include)
#  if __has_include("Arduino.h")
#    define MIDI_ARDUINO_ACTIVE 1
#  else
#    define MIDI_ARDUINO_ACTIVE 0
#  endif
#else
#  define MIDI_ARDUINO_ACTIVE 0
#endif

#if MIDI_ARDUINO_ACTIVE
#  include "Arduino.h"
#endif

#define MIDI_ACTIVE true

#define NAMESPACE_ACTIVE true
#if !defined(LOG_OUT) && MIDI_ARDUINO_ACTIVE
#define LOG_OUT Serial
#endif

#if defined(ESP32) 
#  define MIDI_BLE_ACTIVE true
#  define APPLE_MIDI_ACTIVE true
#  define MDNS_ACTIVE true
#  define UDP_ACTIVE true
#  define TCP_ACTIVE true
#elif defined(ESP8266)
#  define MIDI_BLE_ACTIVE false
#  define APPLE_MIDI_ACTIVE false
#  define MDNS_ACTIVE false
#  define UDP_ACTIVE false
#  define TCP_ACTIVE false
#elif defined(ARDUINO_ARCH_MBED)
#  define APPLE_MIDI_ACTIVE true
#  define MIDI_BLE_ACTIVE false
#  define MDNS_ACTIVE false
#  define UDP_ACTIVE false
#  define TCP_ACTIVE false
#elif defined(ARDUINO)
#  define APPLE_MIDI_ACTIVE true
#  define MIDI_BLE_ACTIVE false
#  define MDNS_ACTIVE false
#  define UDP_ACTIVE true
#  define TCP_ACTIVE true
#else
#  define APPLE_MIDI_ACTIVE false
#  define MIDI_BLE_ACTIVE false
#  define MDNS_ACTIVE false
#  define UDP_ACTIVE false
#  define TCP_ACTIVE false
#endif

