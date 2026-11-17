/**
 * @file usb-device-send.ino
 * @author Phil Schatzmann
 * @brief The microcontroller acts as USB MIDI device and plays some notes:
 * connect it to your computer and select it as MIDI input in your DAW.
 *
 * - ESP32-S2/S3/P4: Tools -> USB Mode: "USB-OTG (TinyUSB)"
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

MidiUsbDevice usbMidi("Arduino MIDI");
uint8_t note = 60;

void setup() {
  usbMidi.begin();
}

void loop() {
  static unsigned long timeout = 0;
  usbMidi.loop();
  if (usbMidi.isConnected() && millis() > timeout) {
    usbMidi.noteOff(note, 0);
    if (++note > 72) note = 60;
    usbMidi.noteOn(note, 100);
    timeout = millis() + 500;
  }
}
