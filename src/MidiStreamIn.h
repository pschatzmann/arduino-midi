#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE
#include "Stream.h"
#include "MidiCommon.h"
#include "MidiEventHandler.h"

namespace midi {

#define BUFFER_LEN 30

/***************************************************/
/*! \class MidiStreamIn
    \brief Input of Midi Messages from the Aruduino 
    HardwareSerial port. You need to give the default
    MidiEventHandler or your own subclassed
    implementation in the constructor to process
     the midi records.

    You also need to call the loop method in the loop
    of your Arduino sketch.

    by Phil Schatzmann
*/
/***************************************************/
class MidiStreamIn : public MidiCommon {
    public:
        MidiStreamIn(Stream &stream, MidiEventHandler &handler);
        void loop();
    protected:
       int getLastStatusPos(uint8_t *buffer, int endPos);
        Stream *pStream;
        MidiEventHandler *pHandler;
        uint8_t buffer[BUFFER_LEN];
        int startPos;
};

} // namespace

#endif
