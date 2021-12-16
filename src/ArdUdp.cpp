
#include <ArdMidiUdp.h>
#ifdef ARDUPD_H

namespace midi {

const char* APP_ArdMidiUdp = "ArdMidiUdp";

ArdMidiUdp :: ArdMidiUdp(char* addessStr,int targetPort) {
    this->isValidHostFlag = WiFi.hostByName(addessStr, targetUdpAddress);
    if (!this->isValidHostFlag){
        ESP_LOGE(APP_ArdMidiUdp, "x%x, Could not resolve host %s ", __func__, addessStr);
    }
    this->targetPort = targetPort;    
}

ArdMidiUdp :: ArdMidiUdp(IPAddress address,int targetPort) {
    this->targetUdpAddress = address;
    this->targetPort = targetPort;    
}

bool ArdMidiUdp :: isValidHost() {
    return this->isValidHostFlag;
}

size_t ArdMidiUdp :: write(const uint8_t * buffer, size_t size ) {
    size_t result = 0;
    if (this->beginPacket(targetUdpAddress, targetPort) == 1){
        result = WiFiUDP::write(buffer, size);
        if (result>0){
            bool packetOk = this->endPacket();
            ESP_LOGD(APP_ArdMidiUdp, "x%x, Number of bytes have %s been sent out: %d ", __func__, packetOk?"":"not", result);
        }
        //this->flush();
    } else {
        ESP_LOGD(APP_ArdMidiUdp, "x%x, beginPacked has failed ", __func__);
        this->isValidHostFlag = false;
    }
    return result;
}

} // stk

#endif