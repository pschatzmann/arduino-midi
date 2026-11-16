#pragma once
#include <stdint.h>
#include "ConfigMidi.h"
#include "MidiAction.h"
#if MIDI_ACTIVE

namespace midi {

/***************************************************/
/*! \class MidiCallbackAction
    \brief MidiAction which can be defined with the
    help of callback methods. Events without a callback are ignored.
    If no 14 bit pitch bend callback is defined, pitch bend is reported
    with 7 bit resolution to the onPitchBend callback.

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

        virtual void onPitchBend14(uint8_t channel, uint16_t value) {
            if (callbackOnPitchBend14!=nullptr) callbackOnPitchBend14(channel, value);
            else MidiAction::onPitchBend14(channel, value);
        }

        virtual void onProgramChange(uint8_t channel, uint8_t program) {
            if (callbackOnProgramChange!=nullptr) callbackOnProgramChange(channel, program);
        }

        virtual void onChannelPressure(uint8_t channel, uint8_t pressure) {
            if (callbackOnChannelPressure!=nullptr) callbackOnChannelPressure(channel, pressure);
        }

        virtual void onPolyPressure(uint8_t channel, uint8_t note, uint8_t pressure) {
            if (callbackOnPolyPressure!=nullptr) callbackOnPolyPressure(channel, note, pressure);
        }

        /// Defines the callback for note on
        virtual void setCallbackOnNoteOn(void (*callback)(uint8_t channel, uint8_t note, uint8_t velocity)) {
            callbackOnNoteOn = callback;
        }

        /// Defines the callback for note off
        virtual void setCallbackOnNoteOff(void (*callback)(uint8_t channel, uint8_t note, uint8_t velocity)) {
            callbackOnNoteOff = callback;
        }

        /// Defines the callback for control change
        virtual void setCallbackOnControlChange(void (*callback)(uint8_t channel, uint8_t controller, uint8_t value)) {
            callbackOnControlChange = callback;
        }

        /// Defines the callback for pitch bend with 7 bit resolution: 0..127, center is 64
        virtual void setCallbackOnPitchBend(void (*callback)(uint8_t channel, uint8_t value)) {
            callbackOnPitchBend = callback;
        }

        /// Pitch bend with full 14 bit resolution: 0..16383, center is 8192
        virtual void setCallbackOnPitchBend14(void (*callback)(uint8_t channel, uint16_t value)) {
            callbackOnPitchBend14 = callback;
        }

        /// Defines the callback for program change
        virtual void setCallbackOnProgramChange(void (*callback)(uint8_t channel, uint8_t program)) {
            callbackOnProgramChange = callback;
        }

        /// Defines the callback for channel pressure
        virtual void setCallbackOnChannelPressure(void (*callback)(uint8_t channel, uint8_t pressure)) {
            callbackOnChannelPressure = callback;
        }

        /// Defines the callback for polyphonic key pressure
        virtual void setCallbackOnPolyPressure(void (*callback)(uint8_t channel, uint8_t note, uint8_t pressure)) {
            callbackOnPolyPressure = callback;
        }

        /// Defines the most important callbacks at once: the program change, pressure and
        /// 14 bit pitch bend callbacks can be defined with the individual setters
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
        void (*callbackOnPitchBend14)(uint8_t channel,  uint16_t value) = nullptr;
        void (*callbackOnProgramChange)(uint8_t channel, uint8_t program) = nullptr;
        void (*callbackOnChannelPressure)(uint8_t channel, uint8_t pressure) = nullptr;
        void (*callbackOnPolyPressure)(uint8_t channel, uint8_t note, uint8_t pressure) = nullptr;

};

} // namespace

#endif
