#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE
#include <WiFi.h>
#include "MidiLogger.h"
#include "MidiServer.h"

namespace midi {

/***************************************************/
/*! \class MidiIpServer
    \brief  A simple IP Server which which receives
    and creates MIDI messages

    by Phil Schatzmann
*/
/***************************************************/

class MidiIpServer : public MidiServer {
    public:
        MidiIpServer(MidiAction *action) : MidiServer(action){
        } 

        ~MidiIpServer() {
            if (p_wifi_server!=nullptr){
                delete p_wifi_server;
            }
        }
         
        bool begin(int serverPort=5008){
            MIDI_LOGI( __PRETTY_FUNCTION__);
        
            if (WiFi.status() != WL_CONNECTED){
                MIDI_LOGE("WiFi not connected");
                return false;
            }

            if (p_wifi_server==nullptr){
                p_wifi_server = new WiFiServer(serverPort);
            }
            p_wifi_server->begin();
            MIDI_LOGI("server started on port %d", serverPort);
            return true;
        }

        void end() {
            if (p_wifi_server!=nullptr){
                delete p_wifi_server;
                p_wifi_server = nullptr;
            }
        }

        void loop() {
            if (p_wifi_server!=nullptr){
                if (!client.connected()){
                    client = p_wifi_server->available();
                    if (client.connected()){
                        MIDI_LOGI("MidiIpServer->connected");
                        in.setup(&client, new MidiParser(p_action), true);
                        out.setup(&client);
                        out.resetAllControllers();
                    }
                } else {
                    MIDI_LOGD("MidiIpServer::loop");
                    in.loop();
                }
            }
        }

    protected:
        WiFiServer *p_wifi_server = nullptr;
        WiFiClient client;

};


}

#endif