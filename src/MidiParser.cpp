#include "MidiParser.h"
#include "MidiLogger.h"

namespace midi {

MidiParser::MidiParser(MidiAction *p_MidiAction, int filter_channel){
  begin(p_MidiAction,filter_channel);
};

MidiParser::~MidiParser(){
}

void MidiParser::begin(MidiAction *p_MidiAction, int filter_channel){
  this->p_MidiAction = p_MidiAction;
  this->filter_channel = filter_channel;
}

/**
 * @brief Parse byte stream for ble midi messages
 * @param [in] msg byte stream
 * @param [in] len length of the msg
 */

void MidiParser::parse(uint8_t* msg, uint8_t len){
  MIDI_LOGD( "parse: len: %d", len);

  int pos = 0;
  uint8_t status=0;
  uint8_t channel=0;
  uint8_t p1=0;
  uint8_t p2=0;

  while (pos<len){
    // find status ingnoring headers and timestamps
    while (msg[pos]>>7 == 1) {
      // if next is data we have a status record
      if (pos+1<len && msg[pos+1]>>7 == 0){
        // status: 0b1001 << 4 | channel;
        status = msg[pos] >> 4;  // high 4 bits
        channel = msg[pos] & 0x0F; // lower 4 bits
        p1 = 0;
        p2 = 0;
      }
      pos++;
    }
     
    // process data bytes
    if (msg[pos]>>7 == 0) { // data
      p1 = msg[pos];
      // check if we have 2 data bytes
      if (pos+1<len && msg[pos+1]>>7 == 0) {
        pos++;
        p2 = msg[pos];
        onCommand(channel, status, p1, p2);
      } else {
        onCommand(channel, status, p1, 0);
      }    
    }
    pos++;
  }
}

void MidiParser::onCommand(uint8_t channel, uint8_t status, uint8_t p1,uint8_t p2 ){
  MIDI_LOGD( "onCommand channel:%d, status:%d, p1:%d,  p2:%d", (int)channel, (int)status, (int)p1, (int)p2);
  MIDI_LOGD( "onCommand filtered channel: %d ", filter_channel);
  if (filter_channel==-1 || filter_channel == channel) {
    switch (status) {
      case 0b1001:
        onNoteOn(channel, p1, p2);
        break;
      case 0b1000:
        onNoteOff(channel, p1, p2);
        break;
      case 0b1110:
        onPitchBend(channel, p1);
        break;
      case 0b1011:
        onControlChange(channel, p1, p2);
        break;
      default:
        //MIDI_LOGI( "Unsupported MIDI status: %d",status);
        break;
    }
  }
}

void MidiParser::onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity){
    MIDI_LOGI( "noteOn note:%d, velocity:%d, channel:%d", (int)note, (int)velocity, (int)channel);
    p_MidiAction->onNoteOn(channel, note, velocity);
};

void MidiParser::onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity){
    MIDI_LOGI( "noteOff note:%d, velocity:%d, channel:%d", (int)note, (int)velocity, (int)channel);
    p_MidiAction->onNoteOff(channel, note, velocity);
};
void MidiParser::onControlChange(uint8_t channel, uint8_t controller, uint8_t value){
    MIDI_LOGI( "onControlChange  channel:%d", (int)channel);
    p_MidiAction->onControlChange(channel, controller, value);
};

void MidiParser::onPitchBend(uint8_t channel, uint8_t value){
    MIDI_LOGI( "onPitchBend  channel:%d", (int)channel);
    p_MidiAction->onPitchBend(channel, value);
};


} // namespace

