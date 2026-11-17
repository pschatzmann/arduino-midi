#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE

#include "MidiUsbBase.h"

// Select the USB device implementation:
// 1) Adafruit TinyUSB (e.g. RP2040 with "USB Stack: Adafruit TinyUSB", SAMD, nRF52)
// 2) ESP32 core USBMIDI (ESP32-S2/S3/P4 with "USB Mode: USB-OTG (TinyUSB)")
// 3) MIDIUSB (RP2040 with the default Pico SDK USB stack, Arduino AVR/SAMD):
//    include <MIDIUSB.h> in your sketch before this header
// 4) Linux desktop: ALSA raw MIDI (USB gadget)
#if defined(USE_TINYUSB) && __has_include("Adafruit_TinyUSB.h")
#  include "Adafruit_TinyUSB.h"
// not available e.g. with "Adafruit TinyUSB Host (native)"
#  if defined(CFG_TUD_ENABLED) && CFG_TUD_ENABLED && defined(CFG_TUD_MIDI) && CFG_TUD_MIDI
#    define MIDI_USB_DEVICE_ADAFRUIT 1
#  endif
#elif defined(ESP32)
#  include "soc/soc_caps.h"
#  include "sdkconfig.h"
#  if SOC_USB_OTG_SUPPORTED && CONFIG_TINYUSB_MIDI_ENABLED
#    include "USB.h"
#    include "USBMIDI.h"
#    define MIDI_USB_DEVICE_ESP32 1
#  endif
#elif __has_include("MIDIUSB.h")
#  include "MIDIUSB.h"
#  define MIDI_USB_DEVICE_MIDIUSB 1
#elif defined(__linux__) && (defined(HOST) || !MIDI_ARDUINO_ACTIVE)
#  include "MidiUsbRawMidi.h"
#  define MIDI_USB_DEVICE_RAWMIDI 1
#endif

#ifndef MIDI_USB_DEVICE_ADAFRUIT
#  define MIDI_USB_DEVICE_ADAFRUIT 0
#endif
#ifndef MIDI_USB_DEVICE_ESP32
#  define MIDI_USB_DEVICE_ESP32 0
#endif
#ifndef MIDI_USB_DEVICE_MIDIUSB
#  define MIDI_USB_DEVICE_MIDIUSB 0
#endif
#ifndef MIDI_USB_DEVICE_RAWMIDI
#  define MIDI_USB_DEVICE_RAWMIDI 0
#endif

#define MIDI_USB_DEVICE_ACTIVE                                \
  (MIDI_USB_DEVICE_ADAFRUIT || MIDI_USB_DEVICE_ESP32 ||       \
   MIDI_USB_DEVICE_MIDIUSB || MIDI_USB_DEVICE_RAWMIDI)

namespace midi {

#if MIDI_USB_DEVICE_RAWMIDI

/***************************************************/
/*! \class MidiUsbDevice
    \brief USB MIDI device on a Linux desktop: the computer acts as USB
    gadget (e.g. Raspberry Pi Zero with the g_midi driver) which provides an
    ALSA raw MIDI device. See MidiUsbRawMidi for the details.
*/
/***************************************************/
class MidiUsbDevice : public MidiUsbRawMidi {
 public:
  /// Constructor: the name is not used on the desktop (it is defined by the
  /// gadget configuration)
  MidiUsbDevice(const char* name = nullptr, MidiAction* action = nullptr,
                int channelFilter = -1)
      : MidiUsbRawMidi(action, channelFilter) {}
};

#elif MIDI_USB_DEVICE_ACTIVE

/***************************************************/
/*! \class MidiUsbDevice
    \brief The microcontroller acts as USB MIDI device (e.g. it is connected
    to a computer which sees it as MIDI interface) which can send and receive
    MIDI messages.

    Supported are
    - Adafruit TinyUSB (RP2040, SAMD, nRF52...): select "USB Stack: Adafruit
      TinyUSB" and include <Adafruit_TinyUSB.h>
    - ESP32-S2/S3/P4: select "USB Mode: USB-OTG (TinyUSB)"
    - MIDIUSB: RP2040 (Pico SDK USB stack), AVR/SAMD with native USB:
      include <MIDIUSB.h> before including MidiUsb.h

    Define the object as global variable (the USB interface must be
    registered before the USB stack is started), call begin() in setup() and
    loop() in the Arduino loop().

    by Phil Schatzmann
*/
/***************************************************/
class MidiUsbDevice : public MidiUsbBase {
 public:
  /// Constructor with the USB product name, the MidiAction which receives
  /// the messages and an optional channel filter (-1 = all channels)
  MidiUsbDevice(const char* name = nullptr, MidiAction* action = nullptr,
                int channelFilter = -1)
      : MidiUsbBase(action, channelFilter)
#if MIDI_USB_DEVICE_ESP32
        , usb_midi(name != nullptr ? name : "ESP32 MIDI")
#endif
  {
    this->name = (char*)name;
  }

#if MIDI_USB_DEVICE_ADAFRUIT
  /// Constructor with an existing Adafruit_USBD_MIDI object (e.g. with
  /// multiple cables)
  MidiUsbDevice(Adafruit_USBD_MIDI& midi, MidiAction* action = nullptr,
                int channelFilter = -1)
      : MidiUsbBase(action, channelFilter) {
    p_midi = &midi;
  }

  /// Provides access to the Adafruit_USBD_MIDI object
  Adafruit_USBD_MIDI& usbMidi() { return *p_midi; }
#endif

#if MIDI_USB_DEVICE_ESP32
  /// Provides access to the ESP32 USBMIDI object
  USBMIDI& usbMidi() { return usb_midi; }
#endif

  /// Starts the USB MIDI device
  bool begin() {
    MIDI_LOGI("MidiUsbDevice::begin");
#if MIDI_USB_DEVICE_ADAFRUIT
    // Manual begin() is required on cores without built-in support
    if (!TinyUSBDevice.isInitialized()) {
      TinyUSBDevice.begin(0);
    }
    if (name != nullptr) {
      TinyUSBDevice.setProductDescriptor(name);
      p_midi->setStringDescriptor(name);
    }
    if (!p_midi->begin()) return false;
    // If already enumerated the new interface is only visible after a
    // re-enumeration
    if (TinyUSBDevice.mounted()) {
      TinyUSBDevice.detach();
      delay(10);
      TinyUSBDevice.attach();
    }
#elif MIDI_USB_DEVICE_ESP32
#  if ARDUINO_USB_MODE
    MIDI_LOGE("MidiUsbDevice: select 'USB Mode: USB-OTG (TinyUSB)'");
    return false;
#  else
    usb_midi.begin();
    USB.begin();
#  endif
#elif MIDI_USB_DEVICE_MIDIUSB
#  if defined(ARDUINO_ARCH_RP2040)
    if (name != nullptr) MidiUSB.setName(name);
    MidiUSB.begin();
#  endif
#endif
    return true;
  }

  /// Starts the USB MIDI device with the indicated MidiAction
  bool begin(MidiAction& action) {
    setMidiAction(action);
    return begin();
  }

  /// Processes the received messages: call it in the Arduino loop()
  bool loop() override {
#if MIDI_USB_DEVICE_ADAFRUIT && defined(TINYUSB_NEED_POLLING_TASK)
    // tud_task() is not called by the core in the background
    TinyUSBDevice.task();
#endif
    setConnectionStatus(mounted() ? Connected : Disconnected);
    return MidiUsbBase::loop();
  }

  /// Sends a USB-MIDI event packet
  bool writePacket(const uint8_t packet[4]) override {
#if MIDI_USB_DEVICE_ADAFRUIT
    if (!TinyUSBDevice.mounted()) return false;
    return p_midi->writePacket(packet);
#elif MIDI_USB_DEVICE_ESP32
    midiEventPacket_t event = {packet[0], packet[1], packet[2], packet[3]};
    return usb_midi.writePacket(&event);
#elif MIDI_USB_DEVICE_MIDIUSB
#  if defined(ARDUINO_ARCH_RP2040)
    return MidiUSB.writePacket(packet);
#  else
    midiEventPacket_t event = {packet[0], packet[1], packet[2], packet[3]};
    MidiUSB.sendMIDI(event);
    MidiUSB.flush();
    return true;
#  endif
#endif
  }

  /// Reads a USB-MIDI event packet: returns false if none is available
  bool readPacket(uint8_t packet[4]) override {
#if MIDI_USB_DEVICE_ADAFRUIT
    return p_midi->readPacket(packet);
#elif MIDI_USB_DEVICE_ESP32
    midiEventPacket_t event;
    if (!usb_midi.readPacket(&event)) return false;
    packet[0] = event.header;
    packet[1] = event.byte1;
    packet[2] = event.byte2;
    packet[3] = event.byte3;
    return true;
#elif MIDI_USB_DEVICE_MIDIUSB
#  if defined(ARDUINO_ARCH_RP2040)
    return MidiUSB.readPacket(packet);
#  else
    midiEventPacket_t event = MidiUSB.read();
    if (event.header == 0) return false;
    packet[0] = event.header;
    packet[1] = event.byte1;
    packet[2] = event.byte2;
    packet[3] = event.byte3;
    return true;
#  endif
#endif
  }

 protected:
#if MIDI_USB_DEVICE_ADAFRUIT
  Adafruit_USBD_MIDI default_midi;
  Adafruit_USBD_MIDI* p_midi = &default_midi;
#elif MIDI_USB_DEVICE_ESP32
  USBMIDI usb_midi;
#endif

  /// Determines if the device is connected to a USB host
  bool mounted() {
#if MIDI_USB_DEVICE_ADAFRUIT
    return TinyUSBDevice.mounted();
#elif MIDI_USB_DEVICE_ESP32
    return tud_mounted();
#elif MIDI_USB_DEVICE_MIDIUSB && defined(ARDUINO_ARCH_RP2040)
    return MidiUSB.connected();
#else
    return true;
#endif
  }
};

#else

/***************************************************/
/*! \class MidiUsbDevice
    \brief USB MIDI device is not available on this platform (or the USB
    stack was not selected): this replacement lets sketches compile.
    begin() reports the problem and nothing is sent or received.
*/
/***************************************************/
class MidiUsbDevice : public MidiUsbBase {
 public:
  MidiUsbDevice(const char* name = nullptr, MidiAction* action = nullptr,
                int channelFilter = -1)
      : MidiUsbBase(action, channelFilter) {}
  bool begin() {
    MIDI_LOGE("MidiUsbDevice: USB MIDI device is not supported on this platform");
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
