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
    \brief Input of Midi Messages from the Aruduino 
    HardwareSerial port. You need to give the default
    MidiParser or your own subclassed
    implementation in the constructor to process
     the midi records.

    You also need to call the loop method in the loop
    of your Arduino sketch.

    by Phil Schatzmann
*/
/***************************************************/
class MidiStreamIn : public MidiCommon {
    public:
        /// Default Constructor
        MidiStreamIn(Stream &stream, MidiAction &action);
        /// Constructor to implement you custom MidiParser
        MidiStreamIn(Stream &stream, MidiParser &handler);
        /// Destructor
        ~MidiStreamIn();
        // Parse/Process the next midi message
        bool loop();
        
    protected:
        friend class MidiServer;
        friend class MidiIpServer;
        friend class MidiUdpServer;

        int getLastStatusPos(uint8_t *buffer, int endPos);
        Stream *pStream = nullptr;
        MidiParser *pHandler = nullptr;
        bool ownsHandler = false;
        uint8_t buffer[BUFFER_LEN];
        int startPos = 0;

        MidiStreamIn() = default;

        void setup(Stream *stream, MidiParser *handler, bool releaseHandler);
};

} // namespace

#endif
