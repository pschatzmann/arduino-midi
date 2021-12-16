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
        MidiStreamOut(Stream &stream);

    protected:
        virtual void writeData(MidiMessage *pMsg, int len);

        Stream *pStream;
        uint8_t buffer[4];
};

} // namespace

#endif
