#pragma once
#include "ConfigMidi.h"
#if MIDI_ACTIVE && TCP_ACTIVE

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "MidiLogger.h"

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

template <class UDPClass = WiFiUDP>
class MidiUdp : public UDPClass {  // EthernetUDP {

 public:
  MidiUdp(char* addessStr, int targetPort) {
    this->isValidHostFlag = WiFi.hostByName(addessStr, targetUdpAddress);
    if (!this->isValidHostFlag) {
      MIDI_LOGE("x%x, Could not resolve host %s ", __func__, addessStr);
    }
    this->targetPort = targetPort;
  }

  MidiUdp(IPAddress address, int targetPort) {
    this->targetUdpAddress = address;
    this->targetPort = targetPort;
  }

  size_t write(const uint8_t* buffer, size_t size) {
    size_t result = 0;
    if (this->beginPacket(targetUdpAddress, targetPort) == 1) {
      result = WiFiUDP::write(buffer, size);
      if (result > 0) {
        bool packetOk = this->endPacket();
        MIDI_LOGD("x%x, Number of bytes have %s been sent out: %d ", __func__,
                  packetOk ? "" : "not", result);
      }
      // this->flush();
    } else {
      MIDI_LOGD("x%x, beginPacked has failed ", __func__);
      this->isValidHostFlag = false;
    }
    return result;
  }

  bool isValidHost() { return this->isValidHostFlag; }

  void setTargetPort(int port) { targetPort = port; }

 protected:
  IPAddress targetUdpAddress;
  int targetPort;
  bool isValidHostFlag;
};

}  // namespace midi

#endif