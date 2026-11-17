#pragma once
#include <stdint.h>

namespace midi {

/***************************************************/
/*! \class MidiUsbPacket
    \brief Conversion between MIDI messages and USB-MIDI 1.0 event packets.

    USB MIDI transfers each message as a 4 byte event packet:
    - byte 0: cable number (high nibble) and Code Index Number (CIN, low nibble)
    - byte 1..3: the MIDI bytes, unused bytes are 0

    The CIN defines how many of the 3 MIDI bytes are valid. This class is
    platform independent (it is also used on the desktop).

    by Phil Schatzmann
*/
/***************************************************/
class MidiUsbPacket {
 public:
  /// Number of valid MIDI bytes in a packet with the indicated Code Index
  /// Number (0 for the reserved CIN 0 and 1)
  static int dataLength(uint8_t cin) {
    static const uint8_t len[16] = {0, 0, 2, 3, 3, 1, 2, 3,
                                    3, 3, 3, 3, 2, 2, 3, 1};
    return len[cin & 0x0F];
  }

  /// Provides the cable number (0..15) of a packet
  static uint8_t cable(const uint8_t packet[4]) { return packet[0] >> 4; }

  /// Provides the Code Index Number of a packet
  static uint8_t cin(const uint8_t packet[4]) { return packet[0] & 0x0F; }

  /// Copies the valid MIDI bytes of the packet to out (max 3 bytes) and
  /// returns their number (0 for an invalid packet)
  static int toBytes(const uint8_t packet[4], uint8_t out[3]) {
    int len = dataLength(cin(packet));
    for (int j = 0; j < len; j++) out[j] = packet[j + 1];
    return len;
  }

  /// Converts a single complete MIDI message (channel, system common or
  /// realtime message: no system exclusive) to a USB-MIDI packet. Returns
  /// false if the message is not supported or incomplete.
  static bool fromMessage(uint8_t cableNo, const uint8_t* msg, int len,
                          uint8_t packet[4]) {
    if (msg == nullptr || len <= 0) return false;
    uint8_t status = msg[0];
    uint8_t cin_value = 0;
    int expected = 0;
    if (status >= 0x80 && status < 0xF0) {
      // channel message: the CIN is the message type
      cin_value = status >> 4;
      expected = (cin_value == 0xC || cin_value == 0xD) ? 2 : 3;
    } else {
      switch (status) {
        case 0xF1:  // MTC quarter frame
        case 0xF3:  // song select
          cin_value = 0x2;
          expected = 2;
          break;
        case 0xF2:  // song position pointer
          cin_value = 0x3;
          expected = 3;
          break;
        case 0xF6:  // tune request
          cin_value = 0x5;
          expected = 1;
          break;
        default:
          if (status >= 0xF8) {
            // system realtime
            cin_value = 0xF;
            expected = 1;
          } else {
            // data byte, sysex or undefined
            return false;
          }
      }
    }
    if (len < expected) return false;
    packet[0] = (uint8_t)((cableNo & 0x0F) << 4) | cin_value;
    packet[1] = status;
    packet[2] = expected > 1 ? (msg[1] & 0x7F) : 0;
    packet[3] = expected > 2 ? (msg[2] & 0x7F) : 0;
    return true;
  }
};

}  // namespace midi
