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
        MidiBleServer(const char* name, MidiBleEventHandler* pEventHandler=nullptr);
        //! Constructor which creates MidiBleEventHandler from MidiBleEventHandler internally
        MidiBleServer(const char* name, MidiAction *MidiAction, int *p_channel = nullptr);
            
        //! Starts the BLE server
        void start(MidiAction &MidiAction);
        void start();
        void  writeData(MidiMessage *pMsg, int len);


    protected:
        MidiBleEventHandler *pEventHandler=nullptr;
        BLEServer *pServer=nullptr;
        BLECharacteristic* pCharacteristic=nullptr;
        const char *name=nullptr;
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
