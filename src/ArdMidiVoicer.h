#pragma once
#include <stdint.h>
#include "ConfigMidi.h"
#if MIDI_ACTIVE

namespace midi {

/***************************************************/
/*! \class MidiVoicer
    \brief Abstract class for a MidiVoicer

    by Phil Schatzmann
*/
/***************************************************/
class MidiVoicer  {
    public:

        virtual void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) = 0;

        virtual void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) = 0;

        virtual void onControlChange(uint8_t channel, uint8_t controller, uint8_t value) = 0;

        virtual void onPitchBend(uint8_t channel, uint8_t value) = 0;
};

} // namespace

#endif
