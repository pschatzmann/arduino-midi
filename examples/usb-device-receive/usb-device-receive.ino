/**
 * @file usb-device-receive.ino
 * @author Phil Schatzmann
 * @brief The microcontroller acts as USB MIDI device and receives the MIDI
 * messages that are sent by the computer.
 *
 * - ESP32-S2/S3/P4: Tools -> USB Mode: "USB-OTG (TinyUSB)". The output is
 *   written to Serial (USB CDC): or use Serial0 with a USB-UART adapter.
 * - RP2040: works with the default "Pico SDK" or the "Adafruit TinyUSB" USB stack
 *
 * @copyright Copyright (c) 2026
 */
#if defined(USE_TINYUSB)
#include <Adafruit_TinyUSB.h>
#elif defined(ARDUINO_ARCH_RP2040)
#include <MIDIUSB.h>
#endif
#include "MidiUsb.h"

MidiCallbackAction action;
MidiUsbDevice usbMidi("Arduino MIDI", &action);

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("onNoteOn: ");
  Serial.println(note);
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("onNoteOff: ");
  Serial.println(note);
}

void setup() {
  Serial.begin(115200);
  action.setCallbacks(onNoteOn, onNoteOff);
  usbMidi.begin();
}

void loop() {
  usbMidi.loop();
}
