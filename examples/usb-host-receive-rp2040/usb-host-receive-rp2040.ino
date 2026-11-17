/**
 * @file usb-host-receive-rp2040.ino
 * @author Phil Schatzmann
 * @brief RP2040/RP2350 as USB host for a USB MIDI keyboard: the native USB
 * port stays a USB device (Serial) and the keyboard is connected with
 * Pico-PIO-USB to 2 GPIO pins (D+ = PIN_USB_HOST_DP, D- = D+ + 1) and 5V.
 *
 * - Install the "Pico PIO USB" library
 * - Tools -> CPU Speed: 120 MHz (or 240 MHz)
 * - Tools -> USB Stack: "Adafruit TinyUSB"
 *
 * The USB host runs on core 1 (setup1 / loop1), the MIDI messages are
 * processed on core 0.
 *
 * @copyright Copyright (c) 2026
 */
#include <Adafruit_TinyUSB.h>
#include <pio_usb.h>
#include "MidiUsb.h"

#ifndef PIN_USB_HOST_DP
#define PIN_USB_HOST_DP 16
#endif

Adafruit_USBH_Host USBHost;
MidiCallbackAction action;
MidiUsbHost usbMidi(&action);

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

// core 1: USB host
void setup1() {
  // PIO USB needs a CPU clock that is a multiple of 12 MHz
  uint32_t cpu_hz = clock_get_hz(clk_sys);
  if (cpu_hz != 120000000UL && cpu_hz != 240000000UL) {
    while (!Serial) delay(10);
    Serial.println("Set the CPU Speed to 120 MHz or 240 MHz");
    while (true) delay(1);
  }
  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = PIN_USB_HOST_DP;
  USBHost.configure_pio_usb(1, &pio_cfg);
  USBHost.begin(1);
}

void loop1() {
  USBHost.task();
}
