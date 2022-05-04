#include "MidiCommon.h"

namespace midi {

MidiCommon::MidiCommon(){
     this->connectionStatus = Unconnected;
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
    uint8_t channel = channelPar != -1 ? channelPar : sendingChannel;
    this->outMessage.status = 0b1001 << 4 | channel;
    this->outMessage.arg1 = note;
    this->outMessage.arg2 = velocity;
    writeData(&outMessage, 2);
}

void MidiCommon :: noteOff(uint8_t note, uint8_t velocity, int8_t channelPar) {
    uint8_t channel = channelPar != -1 ? channelPar : sendingChannel;
    this->outMessage.status = 0b1000 << 4 | channel;
    this->outMessage.arg1 = note;
    this->outMessage.arg2 = velocity;
    writeData(&outMessage, 2);
}

void MidiCommon :: pitchBend(uint16_t value, int8_t channelPar) {
    uint8_t channel = channelPar != -1 ? channelPar : sendingChannel;
    this->outMessage.status = 0b1110 << 4 | channel;
    this->outMessage.arg1 = value >> 8;
    this->outMessage.arg2 = value | 0xFF;
    writeData(&outMessage, 2);
}

void MidiCommon :: channelPressure(uint8_t valuePar, int8_t channel){
    uint8_t value = valuePar & 0b1111111;
    this->outMessage.status = 0b1101 << 4 | channel;
    this->outMessage.arg1 = value ;
    writeData(&outMessage, 1);
}

void MidiCommon :: polyPressure(uint8_t valuePar, int8_t channel){
    uint8_t value = valuePar & 0b1111111;
    this->outMessage.status = 0b1010 << 4 | channel;
    this->outMessage.arg1 = value ;
    writeData(&outMessage, 1);
}

void MidiCommon :: programChange(uint8_t program, int8_t channel){
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

void MidiCommon :: controlChange(uint8_t msg, uint8_t value, int8_t channel){
    this->outMessage.status = 0b1011 << 4 | channel;
    this->outMessage.arg1 = msg;
    this->outMessage.arg2 = value;    
    writeData(&outMessage, 2);
}

float MidiCommon :: noteToFrequency(uint8_t x) {
    float note = x;
    return 440.0 * pow(2.0f, (note-69)/12);
}

uint8_t MidiCommon :: frequencyToNote(float freq) {
    return log(freq/440.0)/log(2) * 12 + 69;
}

void MidiCommon :: writeData(MidiMessage *msg, int len){
}



} // namespace

