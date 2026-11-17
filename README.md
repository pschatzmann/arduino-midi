# Arduino MIDI Library

Quite some time ago, I spent some effort to add Midi support to the [Arduino-STK](https://github.com/pschatzmann/Arduino-STK.git) library.
In the meantime I came to the conclusion that it would be better to have this as a separate functionality, so that it could be used by other projects.

The library supports

- Midi over Arduino Streams 
  - Midi over Serial
  - Midi over Bluetooth Serial
  - Midi over TCP/IP
  - Midi over UDP
  - Files
- Midi over BLE
- Midi over USB (device and host)
- Apple Midi


### Documentation

The basic functionality is based on Arduino Streams (Except the BLE functionality). You give the [MidiStreamOut](https://pschatzmann.github.io/arduino-midi/html/classmidi_1_1_midi_stream_out.html) class an Arduino output stream as argument which will then be used to send/write out the midi data. This gives the flexibility that we can support Files, Serial, IP and UDP and even USB together with the TinyUSB library. 

```
#include "Midi.h"

MidiStreamOut out(Serial);
uint8_t note = 64; // 0 to 127
uint8_t amplitude = 100; // 0 to 127

void setup() {
    Serial.begin(115200);
}

void loop() {
    Serial.println();
    Serial.print("playing ");
    Serial.println(++note);

    out.noteOn( note, amplitude );
    delay(900);
    out.noteOff( note, 20 );
    delay(200);
    if (note>=90) {
      note = 30;
    }
}
```

The same is true for the [MidiStreamIn](https://pschatzmann.github.io/arduino-midi/html/classmidi_1_1_midi_stream_in.html) which is used for receiving and parsing midi messages. There you also need to specify the [MidiAction](https://pschatzmann.github.io/arduino-midi/html/classmidi_1_1_midi_action.html) which implements the actions to be performed on the received events. The loop method processes the next message:

```
#include "Midi.h"

MidiCallbackAction action;
MidiStreamIn in(Serial, action);

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
}

void loop() {
  in.loop();
}

```
### Supported Midi Messages

Channels are numbered 0 to 15. All sending methods accept an optional channel as last parameter: if it is omitted, the default channel (`setDefaultSendingChannel()`, 0 by default) is used. Data values are 0 to 127.

| Message | Sending | Receiving (MidiAction) |
|---------|---------|------------------------|
| Note On / Off | `noteOn(note, velocity)` / `noteOff(note, velocity)` | `onNoteOn()` / `onNoteOff()` |
| Control Change | `controlChange(controller, value)`, `allNotesOff()`, `resetAllControllers()`, `localControl(active)` | `onControlChange()` |
| Program Change | `programChange(program)` | `onProgramChange()` |
| Channel Pressure | `channelPressure(value)` | `onChannelPressure()` |
| Poly Pressure | `polyPressure(note, value, channel)` | `onPolyPressure()` |
| Pitch Bend | `pitchBend(value)`: 0 to 16383, center 8192 | `onPitchBend14()`: 0 to 16383, or `onPitchBend()`: 0 to 127, center 64 |

Notes:

- A Note On with a velocity of 0 is reported as `onNoteOff()`.
- `onNoteOn()`, `onNoteOff()`, `onControlChange()` and `onPitchBend()` must be implemented by a MidiAction, the other methods are optional. By default `onPitchBend14()` forwards the pitch bend with 7 bit resolution to `onPitchBend()`.
- With the `MidiCallbackAction` you can define the callbacks with `setCallbacks()` or the individual setters (e.g. `setCallbackOnProgramChange()`, `setCallbackOnPitchBend14()`).
- The parser supports running status and messages which are split over multiple reads. System exclusive, system common and realtime messages are skipped.
- `polyPressure(value)` without a note is deprecated: it does not send anything.
- `MidiCommon::noteToFrequency()` and `MidiCommon::frequencyToNote()` convert between notes and frequencies in Hz.

### USB MIDI

USB MIDI is provided by `MidiUsb.h` (it is not included by `Midi.h`, so that the USB stack is only used when needed). Both USB roles are supported:

- `MidiUsbDevice`: the board is a USB MIDI device which is connected to a computer
- `MidiUsbHost`: the board is a USB host for a USB MIDI keyboard or controller

Both classes provide the regular sending methods (`noteOn()`, `controlChange()`...) and forward the received messages to a `MidiAction`. Call `loop()` in the Arduino loop. `isConnected()` tells you whether the USB connection is active.

```
#include "MidiUsb.h"

MidiCallbackAction action;
MidiUsbDevice usbMidi("Arduino MIDI", &action);  // or: MidiUsbHost usbMidi(&action);

void setup() {
  action.setCallbacks(onNoteOn, onNoteOff);
  usbMidi.begin();
}

void loop() {
  usbMidi.loop();
}
```

| Platform | MidiUsbDevice | MidiUsbHost |
|----------|---------------|-------------|
| ESP32-S2/S3/P4 | Tools - USB Mode: "USB-OTG (TinyUSB)" | Arduino core >= 3.3, USB Mode: "USB-OTG (TinyUSB)": begin() starts the USB host |
| RP2040/RP2350 | default "Pico SDK" stack: `#include <MIDIUSB.h>` before MidiUsb.h; or "Adafruit TinyUSB" stack: `#include <Adafruit_TinyUSB.h>` | "Adafruit TinyUSB" stack with Pico-PIO-USB (or "Adafruit TinyUSB Host (native)"): you set up and run `Adafruit_USBH_Host` yourself |
| Other Adafruit TinyUSB boards (SAMD, nRF52...) | `#include <Adafruit_TinyUSB.h>` | - |
| AVR/SAMD with native USB (MIDIUSB library) | `#include <MIDIUSB.h>` before MidiUsb.h | - |
| Linux desktop | ALSA raw MIDI of the USB gadget (e.g. Raspberry Pi Zero with g_midi) | ALSA raw MIDI: `begin()` uses the first device, `begin("Keystation")` selects by card name, `begin("/dev/snd/midiC1D0")` by device file |

On the Linux desktop no additional library is needed (the `/dev/snd/midiC*D*` device files are used) and unplugged devices are reconnected automatically. On other platforms the classes compile, but `begin()` reports that USB MIDI is not supported.

See the examples `usb-device-send`, `usb-device-receive`, `usb-host-receive` and `usb-host-receive-rp2040`.

### Server Classes

We also provide some higher level API  "Server" classes which can be used for both, input and output: MidiServer, MidiIpServer, AppleMidiServer...

Here is the generated [Class Documentation](https://pschatzmann.github.io/arduino-midi/html/annotated.html). 
You can find [further information in my blogs](https://www.pschatzmann.ch/home/tag/midi/).

### Logging

The library provides some logging functionality. By default the log level is set to MidiWarning. You can change the level with 
```
MidiLogLevel = MidiDebug; // or MidiInfo, MidiWarning, MidiError
```
On Arduino the log is written to `Serial` (the output is defined by `LOG_OUT` in ConfigMidi.h). On the desktop it is written to stdout.

### Namespace

All the midi classes are defined using the midi namespace. If you include Midi.h the using namespace is already defined. However, if you include the individual class specific header files you need to add a using namespace midi; in your sketch.

### Class Templates / Alternatives Networking Stacks

The following classes support class templates for selecting the networking stack:

- ```AppleMidiServer``` (= ```AppleMidiServer<WiFiUDP>``` )
- ```MidiIpServer``` (= ```MidiIpServer<WiFiServer,WiFiClient>``` )
- ```MidiUdp``` (= ```MidiUdp<WiFiUDP>``` )
- ```MidiUdpServer``` (= ```MidiUdpServer<WiFiUDP>``` )

By default the WiFi classes are used. If you want to use it with Ethernet please use the corresponding class template parameters:

- ```AppleMidiServer<EthernetUDP>```
- ```MidiIpServer<EthernetServer, EthernetClient>```
- ```MidiUdp<EthernetUDP>```
- ```MidiUdpServer<EthernetUDP>```


### Installation in Arduino

You can download the library as zip and call include Library -> zip library. Or you can git clone this project into the Arduino libraries folder e.g. with

```
cd  ~/Documents/Arduino/libraries
git clone https://github.com/pschatzmann/arduino-midi.git

```
I recommend to use git because you can easily update to the latest version just by executing the git pull command in the project folder.

### Unit Tests

The platform independent core (parser, message generation, logging) can be tested on the desktop with CMake:

```
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The Stream based classes (`MidiStreamIn`, `MidiStreamOut`) are only tested when the [Arduino Emulator](https://github.com/pschatzmann/Arduino-Emulator) is available: add `-DMIDI_USE_ARDUINO_EMULATOR=ON` (it is downloaded automatically, or use `-DARDUINO_EMULATOR_DIR=<path>` for a local copy).
