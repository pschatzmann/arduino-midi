#pragma once
#include "ConfigMidi.h"

#if MIDI_BLE_ACTIVE

#include "Arduino.h"
#include "MidiAction.h"
#include "MidiParser.h"
#include "BLECharacteristic.h"
#include "MidiLogger.h"

namespace midi {

/***************************************************/
/*! \class MidiBleParser
    \brief  A simple Midi Parser for BLE Midi messages
    that calls the corresponding events. 
  
    In this implementation the handler just passes the noteOn 
    and noteOff to the MidiAction.
  
    http://www.hangar42.nl/wp-content/uploads/2017/10/BLE-MIDI-spec.pdf

    by Phil Schatzmann
*/
/***************************************************/
class MidiBleParser 
: public BLECharacteristicCallbacks , public  MidiParser {
    public:
        MidiBleParser(MidiAction *MidiAction, int channelFilter = -1 );
        virtual ~MidiBleParser();
        virtual void onRead(BLECharacteristic* pCharacteristic);
	    virtual void onWrite(BLECharacteristic* pCharacteristic);

};


} // namespace

#endif
