#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE && TCP_ACTIVE

#include <WiFi.h>
#include "MidiLogger.h"
#include "MidiUdp.h"
#include "MidiServer.h"

namespace midi {

/***************************************************/
/*! \class MidiUdpServer
    \brief  A simple UDP Server which receives
    and creates MIDI messages

    by Phil Schatzmann
*/
/***************************************************/

template <class UDPClass = WiFiUDP>
class MidiUdpServer : public MidiServer {
    public:
        /// Constructor with the MidiAction which receives the messages
        MidiUdpServer(MidiAction *action):MidiServer(action){
        } 

        ~MidiUdpServer() {
            if (udp!=nullptr){
                delete udp;
                udp = nullptr;
            }
        }
         
        /// Listens on serverPort and sends to the indicated address (the target port is updated from the first received packet)
        bool begin(IPAddress ip, int serverPort=5008){
            MIDI_LOGI( __PRETTY_FUNCTION__);
        
            if (WiFi.status() != WL_CONNECTED){
                MIDI_LOGE("WiFi not connected");
                return false;
            }

            if (udp == nullptr) {
                udp = new MidiUdp<UDPClass>(ip, serverPort);
            }
            // listen for incoming packets
            udp->begin(serverPort);
            in.setup(udp, new MidiParser(p_action), true);
            out.setup(udp);
            return true;
        }

        /// Processes the received packets: call it in the Arduino loop()
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
        MidiUdp<UDPClass> *udp=nullptr;
        bool is_connected = false;
        int remote_port = 0;

};


}

#endif