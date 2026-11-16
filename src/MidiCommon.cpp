#include "MidiCommon.h"
#include "MidiLogger.h"
#include <math.h>

namespace midi {

MidiCommon::MidiCommon(){
     this->connectionStatus = Unconnected;
     this->pMidiAction = nullptr;
     this->outMessage = MidiMessage{};
     this->name = nullptr;
     this->timestampLow = 0;
     this->timestampHigh = 0;
 }

void MidiCommon :: setMidiAction(MidiAction &MidiAction) {
    this->pMidiAction = &MidiAction;
}

void MidiCommon :: setDefaultSendingChannel(int8_t channel) {
    this->sendingChannel = channel;
}

void MidiCommon :: setFilterReceivingChannel(int channel){
    this->receivingChannel = channel;
}

void MidiCommon ::  updateTimestamp(MidiMessage *pMsg) {
    timestampLow++;
    if (timestampLow > 0b01111111 ){
        timestampLow = 0;
        timestampHigh++;
        if (timestampHigh > 0b00111111){
            timestampHigh = 0;
        }
    }
    pMsg->timestampHigh = 0b10000000 + timestampHigh;
    pMsg->timestampLow = 0b10000000 + timestampLow;
}


void MidiCommon :: noteOn(uint8_t note, uint8_t velocity, int8_t channelPar) {
    uint8_t channel = getChannel(channelPar);
    this->outMessage.status = 0b1001 << 4 | channel;
    this->outMessage.arg1 = note & 0b1111111;
    this->outMessage.arg2 = velocity & 0b1111111;
    writeData(&outMessage, 2);
}

void MidiCommon :: noteOff(uint8_t note, uint8_t velocity, int8_t channelPar) {
    uint8_t channel = getChannel(channelPar);
    this->outMessage.status = 0b1000 << 4 | channel;
    this->outMessage.arg1 = note & 0b1111111;
    this->outMessage.arg2 = velocity & 0b1111111;
    writeData(&outMessage, 2);
}

void MidiCommon :: pitchBend(uint16_t value, int8_t channelPar) {
    uint8_t channel = getChannel(channelPar);
    this->outMessage.status = 0b1110 << 4 | channel;
    // value: 0..16383, center 8192: LSB first, then MSB (7 bits each)
    if (value > 0x3FFF) value = 0x3FFF;
    this->outMessage.arg1 = value & 0x7F;
    this->outMessage.arg2 = (value >> 7) & 0x7F;
    writeData(&outMessage, 2);
}

void MidiCommon :: channelPressure(uint8_t valuePar, int8_t channelPar) {
    uint8_t channel = getChannel(channelPar);
    uint8_t value = valuePar & 0b1111111;
    this->outMessage.status = 0b1101 << 4 | channel;
    this->outMessage.arg1 = value ;
    writeData(&outMessage, 1);
}

void MidiCommon :: polyPressure(uint8_t valuePar, int8_t channelPar) {
    // poly pressure requires a note: use the note-specific variant
    MIDI_LOGW("polyPressure without note is not supported: use polyPressure(note, value, channel)");
}

void MidiCommon :: polyPressure(uint8_t note, uint8_t valuePar, int8_t channelPar) {
    uint8_t channel = getChannel(channelPar);
    this->outMessage.status = 0b1010 << 4 | channel;
    this->outMessage.arg1 = note & 0b1111111;
    this->outMessage.arg2 = valuePar & 0b1111111;
    writeData(&outMessage, 2);
}

void MidiCommon :: programChange(uint8_t program, int8_t channelPar) {
    uint8_t channel = getChannel(channelPar);
    uint8_t value = program & 0b1111111;
    this->outMessage.status = 0b1100 << 4 | channel;
    this->outMessage.arg1 = value ;
    writeData(&outMessage, 1);
}

void MidiCommon :: allNotesOff( int8_t channel){
    controlChange(120,0, channel);
}

void MidiCommon :: resetAllControllers( int8_t channel){
    controlChange(121,0, channel);
}

void MidiCommon :: localControl( bool active, int8_t channel){
    controlChange(122,active ? 127 : 0, channel);
}

void MidiCommon :: controlChange(uint8_t msg, uint8_t value, int8_t channelPar) {
    uint8_t channel = getChannel(channelPar);
    this->outMessage.status = 0b1011 << 4 | channel;
    this->outMessage.arg1 = msg & 0b1111111;
    this->outMessage.arg2 = value & 0b1111111;
    writeData(&outMessage, 2);
}

float MidiCommon :: noteToFrequency(uint8_t x) {
    float note = x;
    return 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
}

uint8_t MidiCommon :: frequencyToNote(float freq) {
    // round to the nearest note and limit to the valid range 0..127
    if (freq <= 0.0f) return 0;
    float note = roundf(12.0f * log2f(freq / 440.0f) + 69.0f);
    if (note < 0.0f) return 0;
    if (note > 127.0f) return 127;
    return (uint8_t) note;
}

void MidiCommon :: writeData(MidiMessage *msg, int len){
}

void MidiCommon :: write(MidiMessage *msg, int len){
    writeData(msg, len);
}

uint8_t MidiCommon :: getChannel(int8_t ch) {
    // -1 selects the default sending channel; only 0..15 are valid
    uint8_t result_channel = (ch >= 0) ? ch : sendingChannel;
    return result_channel & 0b1111;
}


} // namespace

