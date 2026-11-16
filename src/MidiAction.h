#pragma once
#include <stdint.h>
#include "ConfigMidi.h"
#if MIDI_ACTIVE

namespace midi {

/***************************************************/
/*! \class MidiAction
    \brief Abstract class which receives the parsed Midi events.

    Subclass it and implement at least onNoteOn(), onNoteOff(),
    onControlChange() and onPitchBend(). The other events have an empty
    default implementation which you can override when needed.
    Channels are 0..15, all other values 0..127 unless noted otherwise.
    Note On messages with a velocity of 0 are reported as onNoteOff().

    by Phil Schatzmann
*/
/***************************************************/
class MidiAction  {
    public:
        virtual ~MidiAction() = default;

        /// Note on with a velocity > 0
        virtual void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) = 0;

        /// Note off (also called for a note on with velocity 0)
        virtual void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) = 0;

        /// Control change: see MidiControlChange.h for the controller numbers
        virtual void onControlChange(uint8_t channel, uint8_t controller, uint8_t value) = 0;

        /// Pitch bend with 7 bit resolution (MSB only): 0..127, center is 64.
        /// Called by the default implementation of onPitchBend14()
        virtual void onPitchBend(uint8_t channel, uint8_t value) = 0;

        /// Pitch bend with full 14 bit resolution: 0..16383, center is 8192.
        /// The default implementation forwards the MSB to onPitchBend()
        virtual void onPitchBend14(uint8_t channel, uint16_t value) {
            onPitchBend(channel, (value >> 7) & 0x7F);
        }

        /// Program change: program 0..127
        virtual void onProgramChange(uint8_t channel, uint8_t program) {}

        /// Channel pressure (aftertouch for the whole channel): 0..127
        virtual void onChannelPressure(uint8_t channel, uint8_t pressure) {}

        /// Polyphonic key pressure (aftertouch for an individual note): 0..127
        virtual void onPolyPressure(uint8_t channel, uint8_t note, uint8_t pressure) {}
};

} // namespace

#endif
