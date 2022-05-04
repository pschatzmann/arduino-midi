#pragma once
#include "ConfigMidi.h"
#ifdef MIDI_ACTIVE

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

namespace midi {

/***************************************************/
/*! \class MidiUdp
    \brief Simple UDP wrapper class which sends all 
    packages to the same destination.

    This is useful when the API asks for a stream
    as parameter to output data.

    by Phil Schatzmann

*/
/***************************************************/

class MidiUdp : public WiFiUDP { // EthernetUDP {
    public:
        MidiUdp(char* targetUdpAddressStr,int targetPort);
        MidiUdp(IPAddress targetUdpAddress,int targetPort);
        size_t write(const uint8_t * buffer,size_t size );
        bool isValidHost();
        void setTargetPort(int port);

    protected:
        IPAddress targetUdpAddress;
        int targetPort;
        bool isValidHostFlag;
};

}

#endif