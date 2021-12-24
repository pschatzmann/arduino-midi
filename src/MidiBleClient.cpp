#include "MidiBleClient.h"
#if MIDI_BLE_ACTIVE
#include "MidiLogger.h"

namespace midi {

const char* APP_CLIENT = "MidiBleClient";
MidiBleParser *pEventHandler;


MidiBleClient :: MidiBleClient(const char* name, MidiBleParser* pEventHandler) {
    this->name = name;
    this->connectionStatus = Unconnected;
}

void MidiBleClient :: begin(MidiAction &MidiAction) {
    this->pMidiAction = &MidiAction;
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MidiBleClientAdvertisedDeviceCallbacks(this));
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}


void characteristic_notify_callback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
    if (pEventHandler)
        pEventHandler->parse(pData,length);
}


void MidiBleClient :: begin(BLEAdvertisedDevice *pDevice) {
    this->pDevice = pDevice;
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MidiBleClientCallback(&connectionStatus));
    //pClient->connect(name);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    pClient->connect(pDevice);  

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(MIDI_SERVICE_UUID);
    if (pRemoteService == nullptr) {
        MIDI_LOGE( "Failed to find our service UUID: %s ", MIDI_SERVICE_UUID);
        pClient->disconnect();
        return;
    }

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(MIDI_CHARACTERISTIC_UUID);
    if (pRemoteCharacteristic == nullptr) {
        MIDI_LOGE( "Failed to find our characteristic UUID: %s", MIDI_CHARACTERISTIC_UUID);
        pClient->disconnect();
        return;
    }
    MIDI_LOGD( " - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
        std::string value = pRemoteCharacteristic->readValue();
        MIDI_LOGE( "The characteristic value was: %s",value.c_str());
    }

    pRemoteCharacteristic->registerForNotify(characteristic_notify_callback, true);

}

void MidiBleClient :: writeData(MidiMessage *pMsg, int len) {
    if (pRemoteCharacteristic!=nullptr){
        updateTimestamp(&outMessage);
        uint8_t* cp = (uint8_t*)&outMessage;
        int messageLen = sizeof(MidiMessage);
        switch (len) {
            case 1: {
                pRemoteCharacteristic->writeValue(cp,len-1, false);
                break;
            }
            case 2: {
                pRemoteCharacteristic->writeValue(cp,len, false);
                break;
            }
        }
    }
}


BLEAdvertisedDevice * MidiBleClient :: getBLEAdvertisedDevice() {
    return this->pDevice;
}


// => MidiBleClientCallback

MidiBleClientCallback :: MidiBleClientCallback(ConnectionStatus *pStatus){
    this->pConnectionStatus = pStatus;
}
void MidiBleClientCallback :: onConnect(BLEClient* client) {
    *pConnectionStatus = Connected;
};

void MidiBleClientCallback :: onDisconnect(BLEClient* client) {
    *pConnectionStatus = Disconnected;
}

// => MidiBleClientAdvertisedDeviceCallbacks

MidiBleClientAdvertisedDeviceCallbacks :: MidiBleClientAdvertisedDeviceCallbacks(MidiBleClient *pClient) {
    this->pClient = pClient;
}

void MidiBleClientAdvertisedDeviceCallbacks :: onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(MIDI_SERVICE_UUID))) {
      BLEDevice::getScan()->stop();
      BLEAdvertisedDevice* pDevice = new BLEAdvertisedDevice(advertisedDevice);
      pClient->begin(pDevice);
    }
}


} // namespace

#endif
