#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE

#include <math.h>

#if MIDI_BLE_ACTIVE
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <nvs_flash.h>
#include <esp_bt.h>            // Ard BLE
#include <esp_bt_device.h>     // Ard BLE
#include <esp_bt_main.h>       // Ard BLE
#include <esp_gap_ble_api.h>   // Ard BLE
#include <esp_gatts_api.h>     // Ard BLE
#include <esp_gattc_api.h>     // Ard BLE
#include <esp_gatt_common_api.h>// Ard BLE
#include <esp_err.h>           // Ard ESP-IDF
#include <map>                 // Part of C++ Standard library
#include <sstream>             // Part of C++ Standard library
#include <iomanip>             // Part of C++ Standard library
#include "BLEDevice.h"
#include "BLEClient.h"
#include "BLEUtils.h"
#include "GeneralUtils.h"
#include "BLE2902.h"

#define MIDI_SERVICE_UUID        "03B80E5A-EDE8-4B33-A751-6CE34EC4C700"
#define MIDI_CHARACTERISTIC_UUID "7772E5DB-3868-4112-A1A9-F2669D106BF3"

#endif

//#include "Arduino.h"
#include "MidiParser.h"
#include "MidiAction.h"
#include "MidiControlChange.h"
#include <stdint.h>

namespace midi {

/**
 * @brief The content of the midi message: timestamp, status, arg1 and arg2
 * 
 */
struct MidiMessage {
    /// BLE header byte: 0b10 + high 6 bits of the timestamp
    uint8_t timestampHigh;
    /// BLE timestamp byte: 0b1 + low 7 bits of the timestamp
    uint8_t timestampLow;
    /// Status byte: message type (high nibble) and channel (low nibble)
    uint8_t status;
    /// First data byte
    uint8_t arg1;
    /// Second data byte (not used by program change and channel pressure)
    uint8_t arg2;
};

enum ConnectionStatus {
    Unconnected,
    Connected,
    Disconnected
};

/***************************************************/
/*! \class MidiCommon
    \brief  The common methods provided by all Arduino Midi
    subclasses which can be used to generate Midi messages.
    Channels are 0..15: if the channel is not specified (-1), the default
    sending channel is used. Data values are 0..127; values outside of the
    range are masked (pitch bend is limited to 0..16383).
    Implemented e.g. by:

    - MidiBleServer
    - MidiBleClient
    - MidiStreamOut

    by Phil Schatzmann
*/
/***************************************************/

class MidiCommon {
    public:
        //! Default Constructor
        MidiCommon();

        //! Activates a filter on receiving messages to the indicated channel
        virtual void setFilterReceivingChannel(int channel);

        //! Sets the default channel (0..15) for the sending commands: the default is 0
        virtual void setDefaultSendingChannel(int8_t channel);

        //! Sends a noteOn MIDI command to the output
        virtual void noteOn(uint8_t note, uint8_t velocity, int8_t channel=-1);

        //! Sends a noteOff MIDI command to the output
        virtual void noteOff(uint8_t note, uint8_t velocity, int8_t channel=-1);

        //! Sends a pitchBend MIDI command to the output: value 0..16383, center 8192
        virtual  void pitchBend(uint16_t value, int8_t channel=-1);

        //! Sends a channelPressure (aftertouch for the whole channel) MIDI command to the output
        virtual void channelPressure(uint8_t value, int8_t channel=-1);

        //! Deprecated: poly pressure needs a note - use polyPressure(note, value, channel)
        virtual void polyPressure(uint8_t valuePar, int8_t channel=-1);

        //! Sends a polyPressure (polyphonic aftertouch) MIDI command to the output
        virtual void polyPressure(uint8_t note, uint8_t value, int8_t channel);

        //! Sends a programChange MIDI command to the output: program 0..127
        virtual void programChange(uint8_t program, int8_t channel=-1);

        //! Sends a allNotesOff MIDI command (control change 120) to the output
        virtual void allNotesOff( int8_t channel=-1);

        //! Sends a resetAllControllers MIDI command (control change 121) to the output
        virtual void resetAllControllers( int8_t channel=-1);

        //! Sends a localControl MIDI command (control change 122) to the output
        virtual void localControl( bool active, int8_t channel=-1);

        //! Sends a control change MIDI command to the output: see MidiControlChange.h for the controller numbers
        virtual void controlChange(uint8_t msg, uint8_t value, int8_t channel=-1);

        //! Converts a MIDI note to a frequency in Hz (note 69 = A4 = 440 Hz)
        static float noteToFrequency(uint8_t note);

        //! Converts a frequency (in Hz) to the nearest MIDI note (limited to 0..127)
        static uint8_t frequencyToNote(float freq);

        //! Defines the MidiAction which receives the events of the inbound processing
        virtual void setMidiAction(MidiAction &MidiAction);

        //! Determines the connection status
        virtual ConnectionStatus getConnectionStatus() { return connectionStatus; }

        //! write multiple MidiMessage objects to final output
        void write(MidiMessage *msg, int len);

    protected:
        void setConnectionStatus(ConnectionStatus status) {connectionStatus=status; }
        void updateTimestamp(MidiMessage *pMsg);
        virtual void writeData(MidiMessage *msg, int len);
        uint8_t getChannel(int8_t ch);
        
        ConnectionStatus connectionStatus;
        MidiAction *pMidiAction;
        MidiMessage outMessage;
        int receivingChannel = -1;  
        uint8_t sendingChannel = 0;  
        uint8_t timestampLow;
        uint8_t timestampHigh;
        char *name;
};


} // namespace

#endif
