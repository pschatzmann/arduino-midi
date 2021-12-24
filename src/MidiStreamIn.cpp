
#include "MidiStreamIn.h"
#include "MidiLogger.h"

namespace midi {

MidiStreamIn :: MidiStreamIn(Stream &stream, MidiAction &action){
    MIDI_LOGI( __PRETTY_FUNCTION__);
    setup(&stream, new MidiParser(&action), true);
}

MidiStreamIn :: MidiStreamIn(Stream &stream, MidiParser &handler) {
    MIDI_LOGI( __PRETTY_FUNCTION__);
    setup(&stream, &handler, false);
}

MidiStreamIn :: ~MidiStreamIn(){
    MIDI_LOGI( __PRETTY_FUNCTION__);
    if (ownsHandler&&pHandler!=nullptr){
        delete pHandler;
        pHandler = nullptr;
    }
}

void MidiStreamIn :: setup(Stream *stream, MidiParser *handler, bool releaseHandler) {
    MIDI_LOGI( __PRETTY_FUNCTION__);
    pStream = stream;
    // without this I was getting a 2 sec delay!
    pStream->setTimeout(10);
    // clenaup last handler
    if (pHandler !=nullptr && ownsHandler){
        delete pHandler;
    }
    pHandler = handler;
    startPos = 0;
    ownsHandler = releaseHandler;
}

bool MidiStreamIn :: loop() {
    MIDI_LOGD( __PRETTY_FUNCTION__);
    bool processed = false;
    if (pStream->available()>0){
        int lenRead = pStream->readBytes(buffer+startPos, BUFFER_LEN-startPos);
        if (lenRead>0){
            MIDI_LOGI( "readBytes: %len", lenRead);
            int endPos = startPos+lenRead;
            int lastStatusPos = getLastStatusPos(buffer, endPos);
            if (lastStatusPos>=0) {
                pHandler->parse(buffer, lastStatusPos);
                int lenUnprocessed = endPos-lastStatusPos;
                // move unprocessed bytes to head
                memmove(buffer, buffer+lastStatusPos, lenUnprocessed);
                startPos = lenUnprocessed;
                processed = true;
            }
        }
    }
    return processed;
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

