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
  // getValue() returns a copy: keep it while the data is parsed
  auto value = pCharacteristic->getValue();
  parseBle((const uint8_t*)value.c_str(), value.length());
} 

} // namespace

#endif