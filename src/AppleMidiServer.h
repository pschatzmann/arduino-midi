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
extern AppleMidiServer *SelfAppleMidi;
typedef void* apple_midi_cb_t;

/***************************************************/
/*! \class AppleMidiServer 

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

        /// Defines the dns name
        void setName(const char* name){
            dns_name = name;
        }

        /// Starts the listening 
        bool begin(int control_port=APPLEMIDI_DEFAULT_PORT);
        /// Starts a session with the indicated address
        bool begin(IPAddress adress, int control_port=APPLEMIDI_DEFAULT_PORT, int data_port_opt=-1);
        /// Closes the connections
        void end();
        /// Processing logic to be executed in loop
        bool loop() {
            return tick(millis());
        }

    protected:
        MidiParser apple_event_handler;
        WiFiUDP udpControl;
        WiFiUDP udpData;
        uint8_t rx_buffer[MIDI_BUFFER_SIZE];
        int remote_port;
        bool is_setup = false;
        const char* dns_name = "AppleMidi";
        
        /// process input from the control and the data port
        virtual bool tick(uint32_t timestamp);
        /// MidiCommon implementation
        virtual void writeData(MidiMessage *msg, int len);
        /// Setup MDNS apple-midi service
        virtual void setupMDns(int port);
        /// provides the network address as string
        const char* toStr(IPAddress &adress);
        /// Activate apple midi debug messages
        void setupLogger();
        /// Callback method to parse midi message
        static void applemidi_callback_midi_message_received(uint8_t port, uint32_t timestamp, uint8_t midi_status, uint8_t *remaining_message, size_t len, size_t continued_sysex_pos);
        /// Callback method to send UDP message with the help of the Arduino API
        static int32_t applemidi_if_send_udp_datagram(uint8_t *ip_addr, uint16_t port, uint8_t *tx_data, size_t tx_len);


};

}