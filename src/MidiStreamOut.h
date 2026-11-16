#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE
#include "Stream.h"
#include "MidiCommon.h"

namespace midi {
/***************************************************/
/*! \class MidiStreamOut
    \brief Output of Midi Messages to an Arduino 
    Stream (eg output to Serial, UDP or IP).
    Each message is written with its status byte (no running status).

    by Phil Schatzmann
*/
/***************************************************/
class MidiStreamOut : public MidiCommon {
    public:
        MidiStreamOut() = default;
        /// Constructor with the output stream
        MidiStreamOut(Print &stream);
        /// Call setup when created with empty constructor
        virtual void setup(Print *stream);
        /// Writes the status and len data bytes to the stream
        virtual void writeData(MidiMessage *pMsg, int len);
    protected:

        Print *pStream = nullptr;
        uint8_t buffer[3];
};

} // namespace

#endif
