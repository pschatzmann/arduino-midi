#include "MidiBleServer.h"
#if MIDI_BLE_ACTIVE

namespace midi {

const char* APP_SERVER = "MidiBleServer";

MidiBleServer::MidiBleServer(const char* name, MidiAction *midiAction, int channelFilter):MidiCommon(){
     this->name = name;
     this->pEventHandler = new MidiBleParser(midiAction, channelFilter);
     this->connectionStatus = Unconnected;
}


MidiBleServer::MidiBleServer(const char* name, MidiBleParser* pEventHandler)
 : MidiCommon() {
     this->name = name;
     this->pEventHandler = pEventHandler;
     this->connectionStatus = Unconnected;
 }

void MidiBleServer :: begin() {
    // Create the BLE Device
    BLEDevice::init(this->name);

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MidiBleServerCallback(&connectionStatus));

    //BLEDevice::setEncryptionLevel((esp_ble_sec_act_t)ESP_LE_AUTH_REQ_SC_BOND);

    // Create the BLE Service
    BLEService *pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                        BLEUUID(MIDI_CHARACTERISTIC_UUID),
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_WRITE_NR  |
                        BLECharacteristic::PROPERTY_NOTIFY |
                        BLECharacteristic::PROPERTY_INDICATE
                    );

                    
    if (this->pMidiAction != nullptr) {                
        if (this->pEventHandler == nullptr){
            MIDI_LOGD( "Creating new MidiBleParser for MidiAction");
            this->pEventHandler =  new MidiBleParser(pMidiAction, this->receivingChannel);
        }
        MIDI_LOGD( "Setting callback for characteristic");
        pCharacteristic->setCallbacks(this->pEventHandler);
    }

    //pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setBroadcastProperty(true);

    // Start the service
    pService->start();

    // Start advertising
    // BLESecurity *pSecurity = new BLESecurity();
    // pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    // pSecurity->setCapability(ESP_IO_CAP_NONE);
    // pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(MIDI_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();

    MIDI_LOGD( "%s: started",__func__);

}

void MidiBleServer :: begin(MidiAction &MidiAction) {
	MIDI_LOGD(__func__);
    setMidiAction(MidiAction);
    begin();
}

void MidiBleServer :: writeData(MidiMessage *pMsg, int len) {
    updateTimestamp(&outMessage);

    switch (len) {
        case 1: 
            pCharacteristic->setValue((uint8_t*)&outMessage, sizeof(MidiMessage)-1); 
            pCharacteristic->notify();  
            break;
        case 2: 
            pCharacteristic->setValue((uint8_t*)&outMessage, sizeof(MidiMessage)); 
            pCharacteristic->notify();  
            break;
    }
}


// => MidiBleServerCallback

MidiBleServerCallback :: MidiBleServerCallback(ConnectionStatus *pStatus){
    this->pConnectionStatus = pStatus;
}
void MidiBleServerCallback :: onConnect(BLEServer* pServer) {
	MIDI_LOGD(__func__);
    *pConnectionStatus = Connected;
};

void MidiBleServerCallback :: onDisconnect(BLEServer* pServer) {
	MIDI_LOGD(__func__);
    *pConnectionStatus = Disconnected;
}

} // namespace

#endif
