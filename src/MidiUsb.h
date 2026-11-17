#pragma once
/**
 * @file MidiUsb.h
 * @brief USB MIDI support (device and host). This header is not included by
 * Midi.h, so that the USB stack is only pulled in when it is needed:
 *
 * - MidiUsbDevice: the board is a USB MIDI device (connected to a computer)
 * - MidiUsbHost: the board is a USB host for a USB MIDI keyboard/controller
 *
 * Supported: ESP32-S2/S3/P4 (TinyUSB), RP2040 (Adafruit TinyUSB, Pico SDK
 * MIDIUSB), other Adafruit TinyUSB and MIDIUSB platforms and the Linux
 * desktop (ALSA raw MIDI).
 */
#include "ConfigMidi.h"
#if MIDI_ARDUINO_ACTIVE
#  include "Midi.h"
#else
// desktop without Arduino API: only the platform independent classes
#  include "MidiCommon.h"
#  include "MidiParser.h"
#  include "MidiCallbackAction.h"
#endif
#include "MidiUsbPacket.h"
#include "MidiUsbBase.h"
#include "MidiUsbDevice.h"
#include "MidiUsbHost.h"

#if NAMESPACE_ACTIVE || MIDI_NAMESPACE_ACTIVE
using namespace midi;
#endif
