#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE
#include <WiFi.h>
#include "MidiLogger.h"

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
        MidiServer(MidiAction *action) {
            p_action = action;
        } 
         
        bool begin(Stream &stream){
            MIDI_LOGI( __PRETTY_FUNCTION__);
            in.setup(&stream, new MidiParser(p_action), true);
            out.setup(&stream);
            return true;
        }

        virtual void end() {
        }

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