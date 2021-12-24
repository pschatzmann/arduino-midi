
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
    static uint8_t last_status;
    int size = 0;

    if (pMsg->status != last_status){
        buffer[size] = pMsg->status;
        size++;
    }
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

