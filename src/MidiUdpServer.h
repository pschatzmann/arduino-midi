#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE
#include <WiFi.h>
#include "MidiLogger.h"
#include "MidiUdp.h"

namespace midi {

/***************************************************/
/*! \class MidiUdpServer
    \brief  A simple UDP Server which receives
    and creates MIDI messages

    by Phil Schatzmann
*/
/***************************************************/

class MidiUdpServer : public MidiCommon {
    public:
        MidiUdpServer(MidiAction *action){
            p_action = action;
        } 

        ~MidiUdpServer() {
            if (udp!=nullptr){
                delete udp;
            }
        }
         
        bool begin(IPAddress ip, int serverPort=5008){
            MIDI_LOGI( __PRETTY_FUNCTION__);
        
            if (WiFi.status() != WL_CONNECTED){
                MIDI_LOGE("WiFi not connected");
                return false;
            }

            udp = new MidiUdp(ip, serverPort);
            in.setup(udp, new MidiEventHandler(p_action), true);
            out.setup(udp);
            return true;
        }

        void loop() {
            if (udp!=nullptr){
                int len = udp->parsePacket();
                if (!is_connected){
                    remote_port = udp->remotePort();
                    if (remote_port!=0){
                        is_connected = true;
                        // we might use a different port on the server
                        udp->setTargetPort(remote_port);
                        MIDI_LOGI("setting traget port: %d", remote_port);
                    } 
                }

                if (udp->available()){
                    in.loop();
                }
            }
        }

    protected:
        MidiUdp *udp=nullptr;
        MidiStreamIn in;
        MidiStreamOut out;
        MidiAction* p_action;
        bool is_connected = false;
        int remote_port = 0;

        virtual void writeData(MidiMessage *msg, int len) {
            MIDI_LOGI( __PRETTY_FUNCTION__);
            out.writeData(msg, len);
        }

};


}

#endif