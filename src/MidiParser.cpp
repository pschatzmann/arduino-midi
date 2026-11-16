#include "MidiParser.h"
#include "MidiLogger.h"
#include <stdio.h>
#include <string.h>

namespace midi {

MidiParser::MidiParser(MidiAction *p_MidiAction, int filter_channel){
  begin(p_MidiAction,filter_channel);
};

MidiParser::~MidiParser(){
}

void MidiParser::begin(MidiAction *p_MidiAction, int filter_channel){
  this->p_MidiAction = p_MidiAction;
  this->filter_channel = filter_channel;
  reset();
}

void MidiParser::reset() {
  running_status = 0;
  data_count = 0;
  in_sysex = false;
}

/// Number of data bytes for a channel voice status byte
uint8_t MidiParser::dataLength(uint8_t status) {
  switch (status >> 4) {
    case 0xC:  // program change
    case 0xD:  // channel pressure
      return 1;
    default:
      return 2;
  }
}

/**
 * @brief Parse a standard midi byte stream. The state is kept between calls,
 * so messages which are split over multiple calls are supported.
 * @param [in] msg byte stream
 * @param [in] len length of the msg
 */
void MidiParser::parse(const uint8_t* msg, int len){
  if (msg == nullptr || len <= 0) return;
  // log hex values of msg (limited to the first bytes to protect the stack)
  if (MidiLogLevel==MidiDebug){
    const int max_dump = 32;
    char msg_hex[max_dump*3+4];
    int dump_len = len < max_dump ? len : max_dump;
    for (int j=0;j<dump_len;j++){
      snprintf(&msg_hex[j*3], 4, "%02X ", msg[j]);
    }
    msg_hex[dump_len*3] = 0;
    if (len > max_dump) strcat(msg_hex, "...");
    MIDI_LOGD( "parse: len: %d - %s", len, msg_hex);
  }
  for (int pos = 0; pos < len; pos++) {
    parseByte(msg[pos]);
  }
}

/**
 * @brief Parse a BLE midi packet: header, then [timestamp] status data...
 * A timestamp followed by a data byte indicates running status.
 * @param [in] msg BLE packet
 * @param [in] len length of the msg
 */
void MidiParser::parseBle(const uint8_t* msg, int len){
  if (msg == nullptr || len <= 0) return;
  // the header byte must have the high bit set
  if ((msg[0] & 0x80) == 0) {
    MIDI_LOGW("parseBle: invalid packet");
    return;
  }
  int pos = 1;
  while (pos < len) {
    uint8_t byte = msg[pos++];
    if (byte & 0x80) {
      // timestamp: the next byte is a status byte or data (running status)
      if (pos < len && (msg[pos] & 0x80)) {
        parseByte(msg[pos++]);
      }
    } else {
      parseByte(byte);
    }
  }
}

void MidiParser::parseByte(uint8_t byte) {
  if (byte >= 0xF8) {
    // system realtime: does not affect running status
    return;
  }
  if (byte & 0x80) {
    if (byte == 0xF0) {
      in_sysex = true;
      running_status = 0;
    } else if (byte >= 0xF0) {
      // end of sysex or system common: we do not process the data
      in_sysex = false;
      running_status = 0;
    } else {
      in_sysex = false;
      running_status = byte;
    }
    data_count = 0;
    return;
  }

  // data byte
  if (in_sysex || running_status == 0) return;
  data[data_count++] = byte;
  if (data_count >= dataLength(running_status)) {
    if (data_count == 1) data[1] = 0;
    data_count = 0;
    onCommand(running_status & 0x0F, running_status >> 4, data[0], data[1]);
  }
}

void MidiParser::onCommand(uint8_t channel, uint8_t status, uint8_t p1,uint8_t p2 ){
  MIDI_LOGD( "onCommand channel:%d, status:%d, p1:%d,  p2:%d", (int)channel, (int)status, (int)p1, (int)p2);
  if (filter_channel==-1 || filter_channel == channel) {
    switch (status) {
      case 0b1001:
        // midi on with velocity 0 -> midi off
        if (p2==0)
          onNoteOff(channel, p1, p2);
        else
          onNoteOn(channel, p1, p2);
        break;
      case 0b1000:
        onNoteOff(channel, p1, p2);
        break;
      case 0b1010:
        onPolyPressure(channel, p1, p2);
        break;
      case 0b1011:
        onControlChange(channel, p1, p2);
        break;
      case 0b1100:
        onProgramChange(channel, p1);
        break;
      case 0b1101:
        onChannelPressure(channel, p1);
        break;
      case 0b1110:
        // p1 is the LSB, p2 the MSB
        onPitchBend14(channel, (uint16_t)p2 << 7 | p1);
        break;
      default:
        break;
    }
  }
}

void MidiParser::onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity){
    MIDI_LOGI( "noteOn note:%d, velocity:%d, channel:%d", (int)note, (int)velocity, (int)channel);
    if (p_MidiAction) p_MidiAction->onNoteOn(channel, note, velocity);
};

void MidiParser::onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity){
    MIDI_LOGI( "noteOff note:%d, velocity:%d, channel:%d", (int)note, (int)velocity, (int)channel);
    if (p_MidiAction) p_MidiAction->onNoteOff(channel, note, velocity);
};

void MidiParser::onControlChange(uint8_t channel, uint8_t controller, uint8_t value){
    MIDI_LOGI( "onControlChange channel:%d, controller:%d, value:%d", (int)channel, (int)controller, (int)value);
    if (p_MidiAction) p_MidiAction->onControlChange(channel, controller, value);
};

void MidiParser::onPitchBend(uint8_t channel, uint8_t value){
    MIDI_LOGI( "onPitchBend channel:%d, value:%d", (int)channel, (int)value);
    if (p_MidiAction) p_MidiAction->onPitchBend(channel, value);
};

void MidiParser::onPitchBend14(uint8_t channel, uint16_t value){
    MIDI_LOGI( "onPitchBend14 channel:%d, value:%d", (int)channel, (int)value);
    if (p_MidiAction) p_MidiAction->onPitchBend14(channel, value);
};

void MidiParser::onProgramChange(uint8_t channel, uint8_t program){
    MIDI_LOGI( "onProgramChange channel:%d, program:%d", (int)channel, (int)program);
    if (p_MidiAction) p_MidiAction->onProgramChange(channel, program);
};

void MidiParser::onChannelPressure(uint8_t channel, uint8_t pressure){
    MIDI_LOGI( "onChannelPressure channel:%d, pressure:%d", (int)channel, (int)pressure);
    if (p_MidiAction) p_MidiAction->onChannelPressure(channel, pressure);
};

void MidiParser::onPolyPressure(uint8_t channel, uint8_t note, uint8_t pressure){
    MIDI_LOGI( "onPolyPressure channel:%d, note:%d, pressure:%d", (int)channel, (int)note, (int)pressure);
    if (p_MidiAction) p_MidiAction->onPolyPressure(channel, note, pressure);
};


} // namespace
