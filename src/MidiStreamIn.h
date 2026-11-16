#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE
#include "Stream.h"
#include "MidiCommon.h"
#include "MidiParser.h"

namespace midi {

#define BUFFER_LEN 30

/***************************************************/
/*! \class MidiStreamIn
    \brief Input of Midi Messages from an Arduino
    Stream (e.g. Serial, Bluetooth Serial or a TCP/IP client).
    In the constructor you either give a MidiAction (which is used with
    the default MidiParser) or your own (subclassed) MidiParser to process
    the midi messages. Messages can be split over multiple reads and
    running status is supported.

    You also need to call the loop method in the loop
    of your Arduino sketch.

    by Phil Schatzmann
*/
/***************************************************/
class MidiStreamIn : public MidiCommon {
    public:
        MidiStreamIn() = default;
        /// Constructor with the input stream and the MidiAction (uses the default MidiParser)
        MidiStreamIn(Stream &stream, MidiAction &action);
        /// Constructor with the input stream and your custom MidiParser (which is not deleted)
        MidiStreamIn(Stream &stream, MidiParser &handler);
        /// Destructor
        ~MidiStreamIn();
        /// Reads all available bytes and processes the contained midi messages:
        /// returns true if some data was processed
        bool loop();        
        /// Call setup when created with empty constructor
        void setup(Stream *stream, MidiParser *handler, bool releaseHandler);
    protected:
        Stream *pStream = nullptr;
        MidiParser *pHandler = nullptr;
        bool ownsHandler = false;
        uint8_t buffer[BUFFER_LEN];

};

} // namespace

#endif
