#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE && TCP_ACTIVE

#include <WiFi.h>
#include "MidiLogger.h"
#include "MidiCommon.h"
#include "MidiStreamIn.h"
#include "MidiStreamOut.h"

namespace midi {

/***************************************************/
/*! \class MidiServer
    \brief  A simple Serial Server which which receives
    and creates MIDI messages

    by Phil Schatzmann
*/
/***************************************************/

class MidiServer : public MidiCommon {
    public:
        /// Constructor with the MidiAction which receives the messages
        MidiServer(MidiAction *action) {
            p_action = action;
        } 
         
        /// Uses the indicated stream for receiving and sending Midi messages
        bool begin(Stream &stream){
            MIDI_LOGI( __PRETTY_FUNCTION__);
            in.setup(&stream, new MidiParser(p_action), true);
            out.setup(&stream);
            return true;
        }

        /// Stops the processing
        virtual void end() {
        }

        /// Processes the received messages: call it in the Arduino loop()
        virtual void loop() {
            MIDI_LOGD("MidiIpServer::loop");
            in.loop();
        }

    protected:
        MidiStreamIn in;
        MidiStreamOut out;
        MidiAction* p_action;

        virtual void writeData(MidiMessage *msg, int len) {
            MIDI_LOGI( __PRETTY_FUNCTION__);
            out.writeData(msg, len);
        }

};


}

#endif