#pragma once
#include "ConfigMidi.h"

#if MIDI_BLE_ACTIVE

#include "Arduino.h"
#include "ArdMidiVoicer.h"
#include "ArdMidiEventHandler.h"
#include "BLECharacteristic.h"
#include "esp_log.h"

namespace midi {

/***************************************************/
/*! \class ArdMidiBleEventHandler
    \brief  A simple Midi Parser for BLE Midi messages
    that calls the corresponding events. 
  
    In this implementation the handler just passes the noteOn 
    and noteOff to the MidiVoicer.
  
    http://www.hangar42.nl/wp-content/uploads/2017/10/BLE-MIDI-spec.pdf

    by Phil Schatzmann
*/
/***************************************************/
class ArdMidiBleEventHandler 
: public BLECharacteristicCallbacks , public  MidiEventHandler {
    public:
        ArdMidiBleEventHandler(MidiVoicer *MidiVoicer, int *p_channel = nullptr );
         ~ArdMidiBleEventHandler();
        void onRead(BLECharacteristic* pCharacteristic);
	    void onWrite(BLECharacteristic* pCharacteristic);

};


} // namespace

#endif
