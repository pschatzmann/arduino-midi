#include "MidiBleEventHandler.h"
#if MIDI_BLE_ACTIVE

namespace midi {

const char * APP_EVENT_HDLR = "MidiBleEventHandler";

MidiBleEventHandler::MidiBleEventHandler(MidiAction *p_MidiAction, int *p_channel)
: MidiEventHandler(p_MidiAction, p_channel) {
};

MidiBleEventHandler::~MidiBleEventHandler(){
}

/**
 * @brief Callback function to support a read request.
 * @param [in] pCharacteristic The characteristic that is the source of the event.
 */
void MidiBleEventHandler::onRead(BLECharacteristic* pCharacteristic) {
	MIDI_LOGD( "%s - onRead",__func__);
} // onRead


/**
 * @brief Callback function to support a write request.
 * @param [in] pCharacteristic The characteristic that is the source of the event.
 */
void MidiBleEventHandler::onWrite(BLECharacteristic* pCharacteristic) {
	MIDI_LOGD( "%s, onWrite",__func__);
  const char* str = pCharacteristic->getValue().c_str();
  int len = pCharacteristic->getValue().length();
  parse((uint8_t*)str, len);
} 

} // namespace

#endif