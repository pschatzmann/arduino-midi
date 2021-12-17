
#include "MidiStreamIn.h"

namespace midi {

MidiStreamIn :: MidiStreamIn(Stream &stream, MidiAction &action){
    pStream = &stream;
    // without this I was getting a 2 sec delay!
    pStream->setTimeout(10);
    pHandler = new MidiEventHandler(&action);
    startPos = 0;
    ownsHandler = true;
}

MidiStreamIn :: MidiStreamIn(Stream &stream, MidiEventHandler &handler) {
    pStream = &stream;
    // without this I was getting a 2 sec delay!
    pStream->setTimeout(10);
    pHandler = &handler;
    startPos = 0;
    ownsHandler = false;
}

MidiStreamIn :: ~MidiEventHandler(){
    if (ownsHandler&&pHandler!=nullptr){
        delete pHandler;
        pHandler = nullptr;
    }
}


void MidiStreamIn :: loop() {
    if (pStream->available()>0){
        int lenRead = pStream->readBytes(buffer+startPos, BUFFER_LEN-startPos);
        if (lenRead>0){
            int endPos = startPos+lenRead;
            int lastStatusPos = getLastStatusPos(buffer, endPos);
            if (lastStatusPos>=0) {
                pHandler->parse(buffer, lastStatusPos);
                int lenUnprocessed = endPos-lastStatusPos;
                // move unprocessed bytes to head
                memmove(buffer, buffer+lastStatusPos, lenUnprocessed);
                startPos = lenUnprocessed;
            }
        }
    }
}

int MidiStreamIn :: getLastStatusPos(uint8_t* buffer, int pos){
    while (pos>=0){
        if (buffer[pos]>>7 == 1){
            return pos; 
        }
        pos--;
    }
    return -1;
} 


} // namespace

