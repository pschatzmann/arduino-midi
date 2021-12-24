#include "MidiBleParser.h"
#if MIDI_BLE_ACTIVE

namespace midi {

const char * APP_EVENT_HDLR = "MidiBleParser";

MidiBleParser::MidiBleParser(MidiAction *p_MidiAction, int channelFilter)
: MidiParser(p_MidiAction, channelFilter) {
};

MidiBleParser::~MidiBleParser(){
}

/**
 * @brief Callback function to support a read request.
 * @param [in] pCharacteristic The characteristic that is the source of the event.
 */
void MidiBleParser::onRead(BLECharacteristic* pCharacteristic) {
	MIDI_LOGD( "%s - onRead",__func__);
} // onRead


/**
 * @brief Callback function to support a write request.
 * @param [in] pCharacteristic The characteristic that is the source of the event.
 */
void MidiBleParser::onWrite(BLECharacteristic* pCharacteristic) {
	MIDI_LOGD( "%s, onWrite",__func__);
  const char* str = pCharacteristic->getValue().c_str();
  int len = pCharacteristic->getValue().length();
  parse((uint8_t*)str, len);
} 

} // namespace

#endif