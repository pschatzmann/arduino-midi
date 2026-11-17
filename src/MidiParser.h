#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE

#include "MidiAction.h"

namespace midi {

/***************************************************/
/*! \class MidiParser
    \brief  A streaming Midi Parser which calls the corresponding events.
    It supports Midi byte streams (with running status) via parse() and
    BLE Midi packets via parseBle() and USB-MIDI event packets via parseUsb(). The parser keeps its state between
    calls, so messages can be split over multiple calls.

    Supported channel voice messages: note on/off, poly pressure, control
    change, program change, channel pressure and 14 bit pitch bend. They are
    forwarded to the MidiAction. System exclusive, system common and system
    realtime messages are skipped (realtime messages may also appear in the
    middle of another message).

    If you indicate the channel (0..15) in the constructor or in begin(), it
    is used as filter to process only the messages for the indicated channel.
    You can subclass the parser and override the on...() methods to intercept
    the events.

    http://www.hangar42.nl/wp-content/uploads/2017/10/BLE-MIDI-spec.pdf


    by Phil Schatzmann
*/
/***************************************************/

class MidiParser  {
    public:
        MidiParser() = default;
        /// Constructor with the MidiAction and an optional channel filter (-1 = all channels)
        MidiParser(MidiAction *MidiAction, int filter_channel = -1 );
        virtual ~MidiParser();

        /// Assigns the MidiAction, optionally defines a channel filter (-1 = all channels) and resets the parser state
        void begin(MidiAction *MidiAction, int filter_channel = -1 );

        /// Parse a standard Midi byte stream (supports running status)
        void parse(const uint8_t*  msg, int len);
        /// Parse a BLE Midi packet (header and timestamp bytes are removed)
        void parseBle(const uint8_t*  msg, int len);
        /// Parse USB-MIDI event packets (4 bytes per packet, the cable number is ignored)
        void parseUsb(const uint8_t*  packets, int len);
        /// Parse a single byte of a standard Midi byte stream
        void parseByte(uint8_t byte);
        /// Resets the parser state (e.g. running status)
        void reset();

        /// Called for each complete channel message: dispatches it to the on...() methods
        virtual void onCommand(uint8_t channel, uint8_t status, uint8_t p1,uint8_t p2 );
        /// Note on with velocity > 0
        virtual void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
        /// Note off or note on with velocity 0
        virtual void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
        /// 7 bit pitch bend (0..127): not called by the parser any more, use onPitchBend14()
        virtual void onPitchBend(uint8_t channel, uint8_t value);
        /// 14 bit pitch bend: 0..16383, center is 8192
        virtual void onPitchBend14(uint8_t channel, uint16_t value);
        /// Control change
        virtual void onControlChange(uint8_t channel, uint8_t controller, uint8_t value);
        /// Program change
        virtual void onProgramChange(uint8_t channel, uint8_t program);
        /// Channel pressure
        virtual void onChannelPressure(uint8_t channel, uint8_t pressure);
        /// Polyphonic key pressure
        virtual void onPolyPressure(uint8_t channel, uint8_t note, uint8_t pressure);

    protected:
        MidiAction *p_MidiAction = nullptr;
        int filter_channel = -1;
        // streaming state
        uint8_t running_status = 0;
        uint8_t data[2] = {0, 0};
        uint8_t data_count = 0;
        bool in_sysex = false;

        static uint8_t dataLength(uint8_t status);
};


} // namespace

#endif
