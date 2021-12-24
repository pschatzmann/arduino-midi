#pragma once

#include <WiFi.h>
#include "ConfigMidi.h"
#include "MidiAction.h"
#include "MidiCommon.h"
#include "MidiLogger.h"
#include "apple-midi/applemidi.h"
#include <WiFiUdp.h>
#if MDNS_ACTIVE
#include <ESPmDNS.h>
#endif

#define MIDI_BUFFER_SIZE 80

namespace midi {

class AppleMidiServer;
AppleMidiServer *SelfAppleMidi = nullptr;
typedef void* apple_midi_cb_t;

/***************************************************/
/*! \class AppleMidi 

    \brief A Sender and Receiver which supports
    Apple Midi using the implementation from midibox.
    Apple midi uses UDP on a control and a data port.
    https://github.com/midibox/esp32-idf-applemidi
    
    by Phil Schatzmann
*/
/***************************************************/

class AppleMidiServer : public MidiCommon  {
    public:
        AppleMidiServer() {
            SelfAppleMidi = this;
        };

        AppleMidiServer(MidiAction *action, int midiPort=-1){
            SelfAppleMidi = this;
            apple_event_handler.begin(action, midiPort);
        }

        void setName(const char* name){
            dns_name = name;
        }

        /// Starts the listening 
        bool begin(int control_port=APPLEMIDI_DEFAULT_PORT){
            MIDI_LOGI( __PRETTY_FUNCTION__);
            if (WiFi.status() != WL_CONNECTED){
                MIDI_LOGE("WIFI is not connected");
                return false;
            }
            setupLogger();
            setupMDns(control_port);
            MIDI_LOGI("MIDI using port: %d", control_port);
            applemidi_init((apple_midi_cb_t) applemidi_callback_midi_message_received, (apple_midi_cb_t) applemidi_if_send_udp_datagram);

            // setup udp
            udpControl.begin(control_port);  // control port
            udpData.begin(control_port+1); // data port
            return true;
        }


        /// Starts a session with the indicated address
        bool begin(IPAddress adress, int control_port=APPLEMIDI_DEFAULT_PORT, int data_port_opt=-1){
            MIDI_LOGI( __PRETTY_FUNCTION__);
            if (WiFi.status() != WL_CONNECTED){
                MIDI_LOGE("WIFI is not connected");
                return false;
            }
            setupLogger();
            setupMDns(control_port);
            applemidi_init((apple_midi_cb_t) applemidi_callback_midi_message_received, (apple_midi_cb_t) applemidi_if_send_udp_datagram);
            int data_port = data_port_opt > 0 ? data_port_opt : control_port+1;
            MIDI_LOGI("MIDI using address: %s port: %d",adress.toString().c_str(), control_port);
            // listen for udp on port
            udpControl.begin(control_port);
            udpData.begin(data_port);
            int status = applemidi_start_session(data_port, (uint8_t*) &adress, control_port);

            return status>=0;
        }

        void end(){
            MIDI_LOGI( __PRETTY_FUNCTION__);
            udpControl.stop();
            udpData.stop();
        }

        /// Processing logic to be executed in loop
        bool loop() {
            return tick(millis());
        }

    protected:
        MidiEventHandler apple_event_handler;
        WiFiUDP udpControl;
        WiFiUDP udpData;
        uint8_t rx_buffer[MIDI_BUFFER_SIZE];
        int remote_port;
        bool is_setup = false;
        const char* dns_name = "AppleMidi";
        
        /// process input from the control and the data port
        virtual bool tick(uint32_t timestamp){
            bool active = false;

            // process control port
            int packetSize = udpControl.parsePacket();
            remote_port = udpControl.remotePort();
            if (packetSize>0){
                // We got some control data
                IPAddress remote_address = udpControl.remoteIP();
                int len = udpControl.read(rx_buffer, MIDI_BUFFER_SIZE);
                MIDI_LOGI("control: %d -> %d ",remote_port, len);
                applemidi_parse_udp_datagram((uint8_t*)&remote_address, remote_port, rx_buffer, len, false);
                active = true;
            } 

            // process data port
            packetSize = udpData.parsePacket();
            if (packetSize>0){
                int remote_port = udpData.remotePort();
                IPAddress remote_address = udpData.remoteIP();
                int len = udpData.read(rx_buffer, MIDI_BUFFER_SIZE);
                MIDI_LOGI("data: %d -> %d",remote_port, len);
                applemidi_parse_udp_datagram((uint8_t*) &remote_address, remote_port, rx_buffer, len, true);
                active = true;
            }
            return active;
        }

        /// MidiCommon implementation
        virtual void writeData(MidiMessage *msg, int len){
            MIDI_LOGI( __PRETTY_FUNCTION__);
            applemidi_send_message(remote_port, (uint8_t*) msg, len*sizeof(MidiMessage));
        }

        /// Setup MDNS apple-midi service
        virtual void setupMDns(int port) {
#if MDNS_ACTIVE
            if(MDNS.begin(dns_name)) {
                MDNS.addService("_apple-midi", "_udp", port);
            } else {
                MIDI_LOGE("Error starting mDNS");
            }
#else
            MIDI_LOGW("MDNS has been deactivated");
#endif
        }

        /// Activate apple midi debug messages
        void setupLogger() {
            switch(MidiLogLevel){
                case MidiInfo:
                    applemidi_set_debug_level(2);
                    break;
                case MidiDebug:
                    applemidi_set_debug_level(3);
                    break;
                case MidiWarning:
                    applemidi_set_debug_level(1);
                    break;
                case MidiError:
                    applemidi_set_debug_level(1);
                    break;
            }
        }

        /// Callback method to parse midi message
        static void applemidi_callback_midi_message_received(uint8_t port, uint32_t timestamp, uint8_t midi_status, uint8_t *remaining_message, size_t len, size_t continued_sysex_pos) {
            MIDI_LOGI("applemidi_callback_midi_message_received: port=%d", port);
            // the parser expects a midi message with the status and the parameters
            uint8_t message[len+1];
            message[0]=midi_status;
            memmove(message+1,remaining_message, len);
            SelfAppleMidi->apple_event_handler.parse(message, len+1);
        }

        /// Callback method to send UDP message with the help of the Arduino API
        static int32_t applemidi_if_send_udp_datagram(uint8_t *ip_addr, uint16_t port, uint8_t *tx_data, size_t tx_len){
            MIDI_LOGI( "applemidi_if_send_udp_datagram: port=%d", port);
            IPAddress *p_adr = (IPAddress *) ip_addr;
            WiFiUDP *p_udp = port == SelfAppleMidi->remote_port ? &(SelfAppleMidi->udpControl) :  &(SelfAppleMidi->udpData);
            p_udp->beginPacket(*p_adr, port);
            int32_t result = p_udp->write(tx_data, tx_len);
            p_udp->endPacket();
            return result;
        }

};

}