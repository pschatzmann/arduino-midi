# Arduino MIDI Library

Quite some time ago, I spent some effort to add Midi support to the [Arduino-STK](https://github.com/pschatzmann/Arduino-STK.git) library.
In the meantime I came to the conclusion that it would be better to have this as a separate functionality, so that it could be used by other 
of my projects.

The library supports

- Midi over an Arduino Streams
  - Midi over Serial
  - Midi over Bluetooth Serial
  - Midi over TCP/IP
  - Midi over UDP
- Midi over BLE
- Apple Midi


### Documentation

The basic functionality is based on Arduino Streams (Except the BLE functionality). You give the [MidiStreamOut](https://pschatzmann.github.io/arduino-midi/html/classmidi_1_1_midi_stream_out.html) class an Arduino output stream as argument which will then be used to send/write out the midi data. This gives the flexibility that we can support Files, Serial, IP and UDP. 

```
#include "Midi.h"

MidiStreamOut out(Serial);
uint16_t note = 64; // 0 to 128
uint16_t amplitude = 100; // 0 to 128

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
MidiStreamIn in(Serial, &action);

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
We also provide some higher level API  "Server" classes which can be used for both, input and output: MidiServer, MidiIpServer, AppleMidiServer...

Here is the generated [Class Documentation](https://pschatzmann.github.io/arduino-midi/html/annotated.html). 
You can find [further information in my blogs](https://www.pschatzmann.ch/home/tag/midi/).


### Namespace

All the midi classes are defined using the midi namespace. If you include Midi.h the using namespace is already defined. However, if you include the individual class specific header files you need to add a using namespace midi; in your sketch.



### Installation in Arduino

You can download the library as zip and call include Library -> zip library. Or you can git clone this project into the Arduino libraries folder e.g. with

```
cd  ~/Documents/Arduino/libraries
git clone pschatzmann/arduino-midi.git

```

