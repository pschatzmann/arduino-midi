#pragma once

#define MIDI_ACTIVE true

#ifdef ESP32
#define MIDI_BLE_ACTIVE true
#else
#define MIDI_BLE_ACTIVE false
#endif

