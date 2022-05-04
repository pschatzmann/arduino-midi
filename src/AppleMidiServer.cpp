#include "AppleMidiServer.h"

namespace midi {

AppleMidiServer *SelfAppleMidi = nullptr;

/// Starts the listening 
bool AppleMidiServer ::  begin(int control_port){
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
bool AppleMidiServer :: begin(IPAddress adress, int control_port, int data_port_opt){
    MIDI_LOGI( __PRETTY_FUNCTION__);
    if (WiFi.status() != WL_CONNECTED){
        MIDI_LOGE("WIFI is not connected");
        return false;
    }
    setupLogger();
    setupMDns(control_port);
    applemidi_init((apple_midi_cb_t) applemidi_callback_midi_message_received, (apple_midi_cb_t) applemidi_if_send_udp_datagram);
    int data_port = data_port_opt > 0 ? data_port_opt : control_port+1;
    MIDI_LOGI("MIDI using address: %s port: %d",toStr(adress), control_port);
    // listen for udp on port
    udpControl.begin(control_port);
    udpData.begin(data_port);
    int status = applemidi_start_session(data_port, (uint8_t*) &adress, control_port);

    return status>=0;
}

const char* AppleMidiServer :: toStr(IPAddress &adress){
    static char networ_str[20];
    //adress.toString() is not supported on all environemnts
    sprintf(networ_str,"%u.%u.%u.%u",adress[0],adress[1],adress[2],adress[3]);
    return networ_str;
}

void AppleMidiServer :: end(){
    MIDI_LOGI( __PRETTY_FUNCTION__);
    udpControl.stop();
    udpData.stop();
}

/// process input from the control and the data port
bool AppleMidiServer ::  tick(uint32_t timestamp){
    bool active = false;

    // process control port
    int packetSize = udpControl.parsePacket();
    remote_port = udpControl.remotePort();
    if (packetSize>0){
        // We got some control data
        IPAddress remote_address = udpControl.remoteIP();
        int len = udpControl.read(rx_buffer, MIDI_BUFFER_SIZE);
        MIDI_LOGD("control: %d -> %d ",remote_port, len);
        applemidi_parse_udp_datagram((uint8_t*)&remote_address, remote_port, rx_buffer, len, false);
        active = true;
    } 

    // process data port
    packetSize = udpData.parsePacket();
    if (packetSize>0){
        int remote_port = udpData.remotePort();
        IPAddress remote_address = udpData.remoteIP();
        int len = udpData.read(rx_buffer, MIDI_BUFFER_SIZE);
        MIDI_LOGD("data: %d -> %d",remote_port, len);
        applemidi_parse_udp_datagram((uint8_t*) &remote_address, remote_port, rx_buffer, len, true);
        active = true;
    }
    return active;
}

/// MidiCommon implementation
void AppleMidiServer ::  writeData(MidiMessage *msg, int len){
    MIDI_LOGI( __PRETTY_FUNCTION__);
    applemidi_send_message(remote_port, (uint8_t*) msg, len*sizeof(MidiMessage));
}

/// Setup MDNS apple-midi service
void AppleMidiServer :: setupMDns(int port) {
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
void AppleMidiServer :: setupLogger() {
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
void AppleMidiServer :: applemidi_callback_midi_message_received(uint8_t port, uint32_t timestamp, uint8_t midi_status, uint8_t *remaining_message, size_t len, size_t continued_sysex_pos) {
    MIDI_LOGI("applemidi_callback_midi_message_received: port=%d", port);
    // the parser expects a midi message with the status and the parameters
    uint8_t message[len+1];
    message[0]=midi_status;
    memmove(message+1,remaining_message, len);
    SelfAppleMidi->apple_event_handler.parse(message, len+1);
}

/// Callback method to send UDP message with the help of the Arduino API
int32_t AppleMidiServer :: applemidi_if_send_udp_datagram(uint8_t *ip_addr, uint16_t port, uint8_t *tx_data, size_t tx_len){
    MIDI_LOGD( "applemidi_if_send_udp_datagram: port=%d", port);
    IPAddress *p_adr = (IPAddress *) ip_addr;
    WiFiUDP *p_udp = port == SelfAppleMidi->remote_port ? &(SelfAppleMidi->udpControl) :  &(SelfAppleMidi->udpData);
    p_udp->beginPacket(*p_adr, port);
    int32_t result = p_udp->write(tx_data, tx_len);
    p_udp->endPacket();
    return result;
}


}