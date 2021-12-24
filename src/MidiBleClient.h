#pragma once
#include "ConfigMidi.h"

#if MIDI_BLE_ACTIVE

#include "MidiCommon.h"
#include "MidiBleParser.h"


namespace midi {

/***************************************************/
/*! \class MidiBleClient
    \brief A Bluetooth Low Energy BLE Client which
    can send or receive Bluetooth messages. It
    needs to connect to a running BLE Server.
    
    by Phil Schatzmann
*/
/***************************************************/

class MidiBleClient : public MidiCommon {
    public:
        //! Default constructor
        MidiBleClient(const char* serverName, MidiBleParser* pEventHandler = nullptr);

        //! starts the discover and connects if the serverName was found
        void begin(MidiAction &MidiAction);

        //! connects to the indicated device
        void begin(BLEAdvertisedDevice *pDevice);
        
        //! Processes a message
        void writeData(MidiMessage *pMsg, int len);

        //! determe in the BLEAdvertisedDevice 
        BLEAdvertisedDevice *getBLEAdvertisedDevice();
        
    protected:
        const char *name;
        BLERemoteCharacteristic* pRemoteCharacteristic;
        BLEAdvertisedDevice* pDevice;

};

/**
 * @brief Client callback method which informs about connect 
 * or disconnect events
 * 
 */
class MidiBleClientCallback: public BLEClientCallbacks {
    public:
        MidiBleClientCallback(ConnectionStatus *pStatus);
        void onConnect(BLEClient* pServer);
        void onDisconnect(BLEClient* pServer);

    protected:
        ConnectionStatus *pConnectionStatus;

};

/**
 * @brief BLEAdvertisedDeviceCallbacks for Midi: reports onResult.
 * 
 */
class MidiBleClientAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    public:
        MidiBleClientAdvertisedDeviceCallbacks(MidiBleClient *pClient);
        void onResult(BLEAdvertisedDevice advertisedDevice);
    private:
        MidiBleClient *pClient;
}; 


} // namespace

#endif
