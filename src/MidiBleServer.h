#pragma once
#include "ConfigMidi.h"

#if MIDI_BLE_ACTIVE

#include "MidiCommon.h"
#include "MidiBleParser.h"

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
        MidiBleServer(const char* name, MidiBleParser* pEventHandler=nullptr);
        //! Constructor which creates MidiBleParser from MidiBleParser internally
        MidiBleServer(const char* name, MidiAction *MidiAction, int channelFilter = -1);
            
        //! Starts the BLE server
        void begin(MidiAction &MidiAction);
        void begin();
        void writeData(MidiMessage *pMsg, int len);


    protected:
        MidiBleParser *pEventHandler=nullptr;
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
        void begin(MidiAction &MidiAction);
        void onConnect(BLEServer* pServer);
        void onDisconnect(BLEServer* pServer);

    protected:
        ConnectionStatus *pConnectionStatus;
};

} // namespace

#endif
