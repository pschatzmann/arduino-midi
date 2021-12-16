#pragma once
#include "ConfigMidi.h"

#if MIDI_BLE_ACTIVE

#include "MidiCommon.h"
#include "MidiBleEventHandler.h"

namespace midi {

/***************************************************/
/*! \class MidiBleServer
    \brief A Bluetooth Low Energy BLE Server which
    can send or receive Bluetooth messages.
    
    by Phil Schatzmann
*/
/***************************************************/

class MidiBleServer : public MidiCommon {
    public:
        //! Default constructor
        MidiBleServer(char* name, MidiBleEventHandler* pEventHandler=nullptr);
        
        //! Starts the BLE server
        void start(MidiAction &MidiAction);
        void start();
        void  writeData(MidiMessage *pMsg, int len);


    protected:
        MidiBleEventHandler *pEventHandler;
        BLEServer *pServer;
        BLECharacteristic* pCharacteristic;
        char *name;
};

/***************************************************/
/*! \class MidiBleServerCallback
    \brief Callback method which informs about 
    connect or disconnect.
    
    by Phil Schatzmann
*/
/***************************************************/


class MidiBleServerCallback: public BLEServerCallbacks {
    public:
        MidiBleServerCallback(ConnectionStatus *pStatus);
        void start(MidiAction &MidiAction);
        void onConnect(BLEServer* pServer);
        void onDisconnect(BLEServer* pServer);

    protected:
        ConnectionStatus *pConnectionStatus;
};

} // namespace

#endif
