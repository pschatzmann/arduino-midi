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

    by Phil Schatzmann
*/
/***************************************************/
class MidiStreamOut : public MidiCommon {
    public:
        /// Default Constructor
        MidiStreamOut(Print &stream);

    protected:
        friend class MidiServer;
        friend class MidiIpServer;
        friend class MidiUdpServer;

        MidiStreamOut() = default;

        virtual void writeData(MidiMessage *pMsg, int len);

        virtual void setup(Print *stream);

        Print *pStream;
        uint8_t buffer[4];
};

} // namespace

#endif
