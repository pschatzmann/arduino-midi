# Arduino MIDI Library

Quite some time ago, I spent some effort to add Midi support to the [Arduino-STK](https://github.com/pschatzmann/Arduino-STK.git) library.
In the meantime I came to the conclusion that it would be better to have this as a separate functionality, so that it could be used by other 
of my projects.

The library supports

- Midi over an Arduino Streams (e.g. Serial)
- Midi over TCP/IP
- Midi over Bluetooth
- Midi over BLE


Please note that most of the examples that generate sound are based on __Arduino-STK__!

### Documentation

The functionality is based on Arduino Streams (Except the BLE functionality). You give the MidiStreamOut class an Arduino output stream as argument which will then be used to send/write out the midi data. This gives the flexibility that we can support Files, Serial, IP and UDP. 

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

The same is true for the MidiStreamIn which is used for receiving and parsing midi messages. There you also need to specify the MidiAction which implements the actions to be performed on the received events. The loop method processes the next message:

```
#include "Midi.h"

MyEventHandler handler
MidiStreamIn in(Serial, handler);

void setup() {
  Serial.begin(115200);
}

void loop() {
  in.loop();
}

```

Here is the generated [Class Documentation](https://pschatzmann.github.io/arduino-midi/html/annotated.html). 

### Installation in Arduino

You can download the library as zip and call include Library -> zip library. Or you can git clone this project into the Arduino libraries folder e.g. with

```
cd  ~/Documents/Arduino/libraries
git clone pschatzmann/arduino-midi.git

```

