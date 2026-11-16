
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
    ownsHandler = releaseHandler;
}

bool MidiStreamIn :: loop() {
    MIDI_LOGD( __PRETTY_FUNCTION__);
    bool processed = false;
    // the parser is streaming: we can process all bytes immediately
    int available;
    while ((available = pStream->available())>0){
        // only request what is available to avoid waiting for the timeout
        int lenRead = pStream->readBytes(buffer, available < BUFFER_LEN ? available : BUFFER_LEN);
        if (lenRead<=0) break;
        MIDI_LOGI( "readBytes: %d", lenRead);
        pHandler->parse(buffer, lenRead);
        processed = true;
    }
    return processed;
}


} // namespace

