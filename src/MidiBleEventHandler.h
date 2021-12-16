#pragma once
#include "ConfigMidi.h"

#if MIDI_BLE_ACTIVE

#include "Arduino.h"
#include "MidiAction.h"
#include "MidiEventHandler.h"
#include "BLECharacteristic.h"
#include "MidiLogger.h"

namespace midi {

/***************************************************/
/*! \class MidiBleEventHandler
    \brief  A simple Midi Parser for BLE Midi messages
    that calls the corresponding events. 
  
    In this implementation the handler just passes the noteOn 
    and noteOff to the MidiAction.
  
    http://www.hangar42.nl/wp-content/uploads/2017/10/BLE-MIDI-spec.pdf

    by Phil Schatzmann
*/
/***************************************************/
class MidiBleEventHandler 
: public BLECharacteristicCallbacks , public  MidiEventHandler {
    public:
        MidiBleEventHandler(MidiAction *MidiAction, int *p_channel = nullptr );
        virtual ~MidiBleEventHandler();
        virtual void onRead(BLECharacteristic* pCharacteristic);
	    virtual void onWrite(BLECharacteristic* pCharacteristic);

};


} // namespace

#endif
