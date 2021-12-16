#pragma once
#include "ConfigMidi.h"

#if MIDI_BLE_ACTIVE

#include "ArdMidiCommon.h"
#include "ArdMidiBleEventHandler.h"

namespace midi {

/***************************************************/
/*! \class ArdMidiBleServer
    \brief A Bluetooth Low Energy BLE Server which
    can send or receive Bluetooth messages.
    
    by Phil Schatzmann
*/
/***************************************************/

class ArdMidiBleServer : public ArdMidiCommon {
    public:
        //! Default constructor
        ArdMidiBleServer(char* name, ArdMidiBleEventHandler* pEventHandler=nullptr);
        
        //! Starts the BLE server
        void start(MidiVoicer &MidiVoicer);
        void start();
        void  writeData(MidiMessage *pMsg, int len);


    protected:
        ArdMidiBleEventHandler *pEventHandler;
        BLEServer *pServer;
        BLECharacteristic* pCharacteristic;
        char *name;
};

class ArdMidiBleServerCallback: public BLEServerCallbacks {
    public:
        ArdMidiBleServerCallback(ConnectionStatus *pStatus);
        void start(MidiVoicer &MidiVoicer);
        void onConnect(BLEServer* pServer);
        void onDisconnect(BLEServer* pServer);

    protected:
        ConnectionStatus *pConnectionStatus;
};

} // namespace

#endif
