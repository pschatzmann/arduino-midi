/**
 * @file usb-host-receive.ino
 * @author Phil Schatzmann
 * @brief The microcontroller acts as USB host: connect a USB MIDI keyboard
 * (with an USB OTG adapter) and the received notes are printed.
 *
 * - ESP32-S2/S3/P4 (Arduino core >= 3.3): Tools -> USB Mode: "USB-OTG (TinyUSB)".
 *   The keyboard needs 5V on VBUS (e.g. ESP32-S3-USB-OTG board or a powered
 *   OTG adapter). Use Serial0 with a USB-UART adapter for the output.
 * - Linux desktop (Arduino Emulator): uses the first ALSA raw MIDI device;
 *   use e.g. usbMidi.begin("Keystation") to select the device by name.
 * - RP2040: see the usb-host-receive-rp2040 example
 *
 * @copyright Copyright (c) 2026
 */
#include "MidiUsb.h"

#if defined(ESP32)
#define Output Serial0
#else
#define Output Serial
#endif

MidiCallbackAction action;
MidiUsbHost usbMidi(&action);

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  Output.print("onNoteOn: ");
  Output.println(note);
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  Output.print("onNoteOff: ");
  Output.println(note);
}

void setup() {
  Output.begin(115200);
  action.setCallbacks(onNoteOn, onNoteOff);
  usbMidi.begin();
}

void loop() {
  static bool connected = false;
  usbMidi.loop();
  if (usbMidi.isConnected() != connected) {
    connected = usbMidi.isConnected();
    Output.println(connected ? "MIDI device connected" : "MIDI device disconnected");
    // play a note on the connected device (e.g. a synthesizer)
    if (connected) usbMidi.noteOn(60, 100);
  }
}
