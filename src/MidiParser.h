#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE

#include "MidiAction.h"

namespace midi {

/***************************************************/
/*! \class MidiParser
    \brief  A simple Midi Parser which calls the corresponding events. 
    It supports Midi and BLE Midi messages. The main entry point
    is the parse command which calls the related methods.
  
    In this implementation the handler just passes the noteOn 
    and noteOff to the MidiAction.
  
    If you indicate the channel in the constructor, it is used as filter
    to process only the messages for the indicated channel.

    http://www.hangar42.nl/wp-content/uploads/2017/10/BLE-MIDI-spec.pdf


    by Phil Schatzmann
*/
/***************************************************/

class MidiParser  {
    public:
        MidiParser() = default;
        MidiParser(MidiAction *MidiAction, int filter_channel = -1 );
         ~MidiParser();

        /// Assigns the MidiAction and optinally defines a midi channel
        void begin(MidiAction *MidiAction, int filter_channel = -1 );

        /// Parse a string into midi messages
        void parse(uint8_t*  msg, uint8_t len);
        virtual void onCommand(uint8_t channel, uint8_t status, uint8_t p1,uint8_t p2 );
        virtual void onNoteOn(uint8_t note, uint8_t velocity,uint8_t channel);
        virtual void onNoteOff(uint8_t note, uint8_t velocity,uint8_t channel);
        virtual void onPitchBend( uint8_t value, uint8_t channel);
        virtual void onControlChange( uint8_t controller, uint8_t controllerValue, uint8_t channel);

    protected:
        MidiAction *p_MidiAction = nullptr; 
        int filter_channel = -1;

};


} // namespace

#endif
