#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE

#include "MidiUsbBase.h"

// Select the USB host implementation:
// 1) ESP32-S2/S3/P4 (Arduino core >= 3.3): TinyUSB MIDI host of the core
// 2) Adafruit TinyUSB with MIDI host support (e.g. RP2040 with Pico-PIO-USB
//    or "USB Stack: Adafruit TinyUSB Host")
// 3) Linux desktop: ALSA raw MIDI
#if defined(ESP32) && __has_include("USBHost.h")
#  include "soc/soc_caps.h"
#  include "sdkconfig.h"
#  if SOC_USB_OTG_SUPPORTED && CONFIG_TINYUSB_ENABLED
#    include "USBHost.h"
#    if CFG_TUH_ENABLED && CFG_TUH_MIDI
#      define MIDI_USB_HOST_TINYUSB 1
#      define MIDI_USB_HOST_ESP32 1
#    endif
#  endif
#elif defined(USE_TINYUSB) && __has_include("Adafruit_TinyUSB.h")
#  include "Adafruit_TinyUSB.h"
#  if defined(CFG_TUH_ENABLED) && CFG_TUH_ENABLED && defined(CFG_TUH_MIDI) && CFG_TUH_MIDI
#    define MIDI_USB_HOST_TINYUSB 1
#  endif
#elif defined(__linux__) && (defined(HOST) || !MIDI_ARDUINO_ACTIVE)
#  include "MidiUsbRawMidi.h"
#  define MIDI_USB_HOST_RAWMIDI 1
#endif

#ifndef MIDI_USB_HOST_TINYUSB
#  define MIDI_USB_HOST_TINYUSB 0
#endif
#ifndef MIDI_USB_HOST_ESP32
#  define MIDI_USB_HOST_ESP32 0
#endif
#ifndef MIDI_USB_HOST_RAWMIDI
#  define MIDI_USB_HOST_RAWMIDI 0
#endif

#define MIDI_USB_HOST_ACTIVE (MIDI_USB_HOST_TINYUSB || MIDI_USB_HOST_RAWMIDI)

namespace midi {

#if MIDI_USB_HOST_RAWMIDI

/***************************************************/
/*! \class MidiUsbHost
    \brief USB MIDI host on a Linux desktop: communicates with a USB MIDI
    device (e.g. keyboard) that is plugged into the computer. Call
    begin("Keystation") to select the device by (a part of) its ALSA card
    name, begin("/dev/snd/midiC1D0") to use a device file or begin() for
    the first device. See MidiUsbRawMidi for the details.
*/
/***************************************************/
class MidiUsbHost : public MidiUsbRawMidi {
 public:
  MidiUsbHost(MidiAction* action = nullptr, int channelFilter = -1)
      : MidiUsbRawMidi(action, channelFilter) {}
};

#elif MIDI_USB_HOST_TINYUSB

/***************************************************/
/*! \class MidiUsbHost
    \brief The microcontroller acts as USB host for a USB MIDI device (e.g.
    a keyboard or controller): it receives the MIDI messages of the device
    and can send messages to it. Based on the TinyUSB MIDI host driver.

    - ESP32-S2/S3/P4 (Arduino core >= 3.3, "USB Mode: USB-OTG (TinyUSB)"):
      begin() starts the USB host and loop() calls USBHost.task()
    - RP2040 with Adafruit TinyUSB: you need to set up and run the
      Adafruit_USBH_Host yourself (e.g. Pico-PIO-USB on core 1 with
      USBHost.task() in loop1()).

    Call loop() in the Arduino loop() to process the received messages. The
    device can be plugged in and out at any time: use isConnected() to check
    if a MIDI device is attached.

    by Phil Schatzmann
*/
/***************************************************/
class MidiUsbHost : public MidiUsbBase {
 public:
  /// Constructor with the MidiAction which receives the messages and an
  /// optional channel filter (-1 = all channels)
  MidiUsbHost(MidiAction* action = nullptr, int channelFilter = -1)
      : MidiUsbBase(action, channelFilter) {}

  /// Starts the processing (on the ESP32 the USB host is started as well)
  bool begin() {
    MIDI_LOGI("MidiUsbHost::begin");
#if MIDI_USB_HOST_ESP32
    if (!USBHost.started() && !USBHost.begin()) {
      MIDI_LOGE("MidiUsbHost: USBHost.begin() failed");
      return false;
    }
#endif
    return true;
  }

  /// Starts the processing with the indicated MidiAction
  bool begin(MidiAction& action) {
    setMidiAction(action);
    return begin();
  }

  /// Selects the MIDI interface by TinyUSB index (0..CFG_TUH_MIDI-1).
  /// -1 (the default) uses the first mounted device
  void setDeviceIndex(int idx) { device_index = idx; }

  /// Provides the TinyUSB index of the active MIDI device (-1 if none)
  int activeIndex() { return active_idx; }

  /// Provides the USB address of the active MIDI device (0 if none)
  uint8_t deviceAddress() {
    tuh_itf_info_t info;
    if (active_idx < 0 || !tuh_midi_itf_get_info(active_idx, &info)) return 0;
    return info.daddr;
  }

  /// Number of cables (virtual MIDI ports) of the device for receiving
  uint8_t rxCableCount() {
    return active_idx < 0 ? 0 : tuh_midi_get_rx_cable_count(active_idx);
  }

  /// Number of cables (virtual MIDI ports) of the device for sending
  uint8_t txCableCount() {
    return active_idx < 0 ? 0 : tuh_midi_get_tx_cable_count(active_idx);
  }

  /// Processes the received messages: call it in the Arduino loop()
  bool loop() override {
#if MIDI_USB_HOST_ESP32
    USBHost.task();
#endif
    updateDevice();
    return MidiUsbBase::loop();
  }

  /// Sends a USB-MIDI event packet to the device
  bool writePacket(const uint8_t packet[4]) override {
    if (active_idx < 0) return false;
    if (!tuh_midi_packet_write(active_idx, packet)) {
      // tx buffer is full: send the buffered data and try again
      tuh_midi_write_flush(active_idx);
      if (!tuh_midi_packet_write(active_idx, packet)) return false;
    }
    tuh_midi_write_flush(active_idx);
    return true;
  }

  /// Reads a USB-MIDI event packet from the device
  bool readPacket(uint8_t packet[4]) override {
    if (active_idx < 0) return false;
    return tuh_midi_packet_read(active_idx, packet);
  }

 protected:
  int device_index = -1;
  int active_idx = -1;

  /// Determines the active device and updates the connection status
  void updateDevice() {
    if (active_idx >= 0 && tuh_midi_mounted(active_idx)) return;
    int new_idx = -1;
    if (device_index >= 0) {
      if (device_index < CFG_TUH_MIDI && tuh_midi_mounted(device_index))
        new_idx = device_index;
    } else {
      for (int j = 0; j < CFG_TUH_MIDI; j++) {
        if (tuh_midi_mounted(j)) {
          new_idx = j;
          break;
        }
      }
    }
    if (new_idx != active_idx) {
      if (new_idx >= 0) {
        MIDI_LOGI("MidiUsbHost: MIDI device %d connected", new_idx);
        default_parser.reset();
        if (p_parser != nullptr) p_parser->reset();
        setConnectionStatus(Connected);
      } else {
        MIDI_LOGI("MidiUsbHost: MIDI device %d disconnected", active_idx);
        setConnectionStatus(Disconnected);
      }
      active_idx = new_idx;
    }
  }
};

#else

/***************************************************/
/*! \class MidiUsbHost
    \brief USB MIDI host is not available on this platform (or the USB stack
    was not selected): this replacement lets sketches compile. begin()
    reports the problem and nothing is sent or received.
*/
/***************************************************/
class MidiUsbHost : public MidiUsbBase {
 public:
  MidiUsbHost(MidiAction* action = nullptr, int channelFilter = -1)
      : MidiUsbBase(action, channelFilter) {}
  bool begin() {
    MIDI_LOGE("MidiUsbHost: USB MIDI host is not supported on this platform");
    return false;
  }
  bool begin(MidiAction& action) {
    setMidiAction(action);
    return begin();
  }
  bool writePacket(const uint8_t packet[4]) override { return false; }
  bool readPacket(uint8_t packet[4]) override { return false; }
};

#endif

}  // namespace midi

#endif
