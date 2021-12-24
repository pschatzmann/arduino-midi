#pragma once
#include <stdint.h>
#include "ConfigMidi.h"
#include "MidiAction.h"
#if MIDI_ACTIVE

namespace midi {

/***************************************************/
/*! \class MidiCallbackAction
    \brief MidiAction which can be defined with the
    help of callback methods

    by Phil Schatzmann
*/
/***************************************************/
class MidiCallbackAction : public MidiAction {
    public:

        virtual void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
            if (callbackOnNoteOn!=nullptr) callbackOnNoteOn(channel, note, velocity);
        }

        virtual void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
            if (callbackOnNoteOff!=nullptr) callbackOnNoteOff(channel, note, velocity);
        }

        virtual void onControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
            if (callbackOnControlChange!=nullptr) callbackOnControlChange(channel, controller, value);
        }

        virtual void onPitchBend(uint8_t channel, uint8_t value) {
            if (callbackOnPitchBend!=nullptr) callbackOnPitchBend(channel, value);
        }

        virtual void setCallbackOnNoteOn(void (*callback)(uint8_t channel, uint8_t note, uint8_t velocity)) {
            callbackOnNoteOn = callback;
        }

        virtual void setCallbackOnNoteOff(void (*callback)(uint8_t channel, uint8_t note, uint8_t velocity)) {
            callbackOnNoteOff = callback;
        }

        virtual void setCallbackOnControlChange(void (*callback)(uint8_t channel, uint8_t controller, uint8_t value)) {
            callbackOnControlChange = callback;
        }

        virtual void setCallbackOnPitchBend(void (*callback)(uint8_t channel, uint8_t value)) {
            callbackOnPitchBend = callback;
        }

        virtual void setCallbacks(
                void (*callbackOnNoteOn)(uint8_t channel, uint8_t note, uint8_t velocity),
                void (*callbackOnNoteOff)(uint8_t channel, uint8_t note, uint8_t velocity),
                void (*callbackOnControlChange)(uint8_t channel, uint8_t controller, uint8_t value) = nullptr,
                void (*callbackOnPitchBend)(uint8_t channel, uint8_t value) = nullptr) {
        this->callbackOnNoteOn = callbackOnNoteOn;
        this->callbackOnNoteOff = callbackOnNoteOff;
        this->callbackOnControlChange = callbackOnControlChange;
        this->callbackOnPitchBend = callbackOnPitchBend;

        }

    protected:
        void (*callbackOnNoteOn)(uint8_t channel, uint8_t note, uint8_t velocity) = nullptr;
        void (*callbackOnNoteOff)(uint8_t channel, uint8_t note, uint8_t velocity) = nullptr;
        void (*callbackOnControlChange)(uint8_t channel, uint8_t controller, uint8_t value) = nullptr;
        void (*callbackOnPitchBend)(uint8_t channel,  uint8_t value) = nullptr;

};

} // namespace

#endif
