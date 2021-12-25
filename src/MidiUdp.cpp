#include <MidiUdp.h>
#ifdef MIDI_ACTIVE

#include "MidiLogger.h"

namespace midi {

const char* APP_MidiUdp = "MidiUdp";

MidiUdp :: MidiUdp(char* addessStr,int targetPort) {
    this->isValidHostFlag = WiFi.hostByName(addessStr, targetUdpAddress);
    if (!this->isValidHostFlag){
        MIDI_LOGE( "x%x, Could not resolve host %s ", __func__, addessStr);
    }
    this->targetPort = targetPort;    
}

MidiUdp :: MidiUdp(IPAddress address,int targetPort) {
    this->targetUdpAddress = address;
    this->targetPort = targetPort;    
}

bool MidiUdp :: isValidHost() {
    return this->isValidHostFlag;
}

void MidiUdp :: setTargetPort(int port){
    targetPort = port;   
}

size_t MidiUdp :: write(const uint8_t * buffer, size_t size ) {
    size_t result = 0;
    if (this->beginPacket(targetUdpAddress, targetPort) == 1){
        result = WiFiUDP::write(buffer, size);
        if (result>0){
            bool packetOk = this->endPacket();
            MIDI_LOGD( "x%x, Number of bytes have %s been sent out: %d ", __func__, packetOk?"":"not", result);
        }
        //this->flush();
    } else {
        MIDI_LOGD( "x%x, beginPacked has failed ", __func__);
        this->isValidHostFlag = false;
    }
    return result;
}

} // stk

#endif

