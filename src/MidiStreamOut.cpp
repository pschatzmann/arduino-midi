
#include "MidiStreamOut.h"
#include "MidiLogger.h"

namespace midi {

MidiStreamOut :: MidiStreamOut(Print &stream) {
    setup(&stream);
    //this->setConnectionStatus(stream ? Connected : Unconnected);
}

void MidiStreamOut :: setup(Print *stream){
    pStream = stream;
}

void MidiStreamOut :: writeData(MidiMessage *pMsg, int len) {
    if (pStream == nullptr) {
        MIDI_LOGE("MidiStreamOut: no output stream defined");
        return;
    }
    // we always send the status byte (no running status)
    int size = 0;
    buffer[size++] = pMsg->status;
    if (len>=1){
        buffer[size] = pMsg->arg1;
        size++;
    }
    if (len==2){
        buffer[size] = pMsg->arg2;
        size++;
    }
    pStream->write(buffer,size);
}



} // namespace

