#pragma once
#include "ConfigMidi.h"
#if TCP_ACTIVE
#include <WiFi.h>
#include <WiFiUdp.h>

#include "MidiAction.h"
#include "MidiCommon.h"
#include "MidiLogger.h"
#include "apple-midi/applemidi.h"
#if MDNS_ACTIVE
#include <ESPmDNS.h>
#endif

#define MIDI_BUFFER_SIZE 80

namespace midi {

typedef void* apple_midi_cb_t;

/***************************************************/
/*! \class AppleMidiServer

    \brief A Sender and Receiver which supports
    Apple Midi using the implementation from midibox.
    Apple midi uses UDP on a control and a data port.
    https://github.com/midibox/esp32-idf-applemidi

    by Phil Schatzmann
*/
/***************************************************/
template <class UDPClass = WiFiUDP>
class AppleMidiServer : public MidiCommon {
 public:
  /// Default constructor: define the MidiAction with setMidiAction() if you want to receive messages
  AppleMidiServer() {
    _instance = this;
  };

  /// Constructor with the MidiAction which receives the messages; midiPort is an optional channel filter (-1 = all)
  AppleMidiServer(MidiAction* action, int midiPort = -1) {
    _instance = this;
    apple_event_handler.begin(action, midiPort);
  }

  /// Defines the mDNS name (default "AppleMidi")
  void setName(const char* name) { dns_name = name; }

  /// Starts the server and waits for a remote session: the data port is control_port+1
  bool begin(int control_port = APPLEMIDI_DEFAULT_PORT) {
    MIDI_LOGI(__PRETTY_FUNCTION__);
    setupLogger();
    setupMDns(control_port);
    MIDI_LOGI("MIDI using port: %d", control_port);
    applemidi_init((apple_midi_cb_t)applemidi_callback_midi_message_received,
                   (apple_midi_cb_t)applemidi_if_send_udp_datagram);
    udpControl.begin(control_port);   // control port
    udpData.begin(control_port + 1);  // data port
    return true;
  }

  /// Starts the server and invites the remote Apple Midi peer at the indicated address and control port
  bool begin(IPAddress adress, int control_port = APPLEMIDI_DEFAULT_PORT,
             int data_port_opt = -1) {
    _instance = this;
    MIDI_LOGI(__PRETTY_FUNCTION__);
    setupLogger();
    setupMDns(control_port);
    applemidi_init((apple_midi_cb_t)applemidi_callback_midi_message_received,
                   (apple_midi_cb_t)applemidi_if_send_udp_datagram);
    int data_port = data_port_opt > 0 ? data_port_opt : control_port + 1;
    MIDI_LOGI("MIDI using address: %s port: %d", toStr(adress), control_port);
    udpControl.begin(control_port);
    udpData.begin(data_port);
    // invite the remote peer: the first argument is the internal peer index
    int32_t peer = applemidi_search_free_port();
    if (peer < 1) {
      MIDI_LOGE("No free applemidi peer slot");
      return false;
    }
    uint8_t ip[4];
    toBytes(adress, ip);
    int status = applemidi_start_session(peer, ip, control_port);
    return status >= 0;
  }

  /// Stops the UDP communication
  void end() {
    MIDI_LOGI(__PRETTY_FUNCTION__);
    udpControl.stop();
    udpData.stop();
  }

  /// Processes the received packets and flushes the output: call it in the Arduino loop(). Returns true if data was received
  bool loop() { return tick(millis()); }

 protected:
  MidiParser apple_event_handler;
  UDPClass udpControl;
  UDPClass udpData;
  uint8_t rx_buffer[MIDI_BUFFER_SIZE];
  int remote_port = 0;
  bool is_setup = false;
  const char* dns_name = "AppleMidi";
  static AppleMidiServer<UDPClass>* _instance;

  virtual bool tick(uint32_t timestamp) {
    bool active = false;
    // flush output buffers and synchronize clocks
    applemidi_tick();
    int packetSize = udpControl.parsePacket();
    remote_port = udpControl.remotePort();
    if (packetSize > 0) {
      uint8_t ip[4];
      toBytes(udpControl.remoteIP(), ip);
      int len = udpControl.read(rx_buffer, MIDI_BUFFER_SIZE);
      MIDI_LOGD("control: %d -> %d ", remote_port, len);
      if (len > 0)
        applemidi_parse_udp_datagram(ip, remote_port, rx_buffer, len, false);
      active = true;
    }
    packetSize = udpData.parsePacket();
    if (packetSize > 0) {
      int data_remote_port = udpData.remotePort();
      uint8_t ip[4];
      toBytes(udpData.remoteIP(), ip);
      int len = udpData.read(rx_buffer, MIDI_BUFFER_SIZE);
      MIDI_LOGD("data: %d -> %d", data_remote_port, len);
      if (len > 0)
        applemidi_parse_udp_datagram(ip, data_remote_port, rx_buffer, len,
                                     true);
      active = true;
    }
    return active;
  }

  virtual void writeData(MidiMessage* msg, int len) {
    MIDI_LOGI(__PRETTY_FUNCTION__);
    // send the status and len data bytes to all connected peers
    // (peer 0 is ourself)
    for (uint8_t peer = 1; peer < APPLEMIDI_MAX_PEERS; peer++) {
      applemidi_peer_t* info = applemidi_peer_get_info(peer);
      if (info != nullptr && info->data_port > 0) {
        applemidi_send_message(peer, &msg->status, len + 1);
      }
    }
  }

  /// IPAddress is not a plain byte array: copy the IPv4 bytes
  static void toBytes(const IPAddress& address, uint8_t* ip) {
    for (int j = 0; j < 4; j++) ip[j] = address[j];
  }

  virtual void setupMDns(int port) {
#if MDNS_ACTIVE
    if (MDNS.begin(dns_name)) {
      MDNS.addService("_apple-midi", "_udp", port);
    } else {
      MIDI_LOGE("Error starting mDNS");
    }
#else
    MIDI_LOGW("MDNS has been deactivated");
#endif
  }

  const char* toStr(IPAddress& adress) {
    static char networ_str[20];
    sprintf(networ_str, "%u.%u.%u.%u", adress[0], adress[1], adress[2],
            adress[3]);
    return networ_str;
  }

  void setupLogger() {
    switch (MidiLogLevel) {
      case MidiInfo:
        applemidi_set_debug_level(2);
        break;
      case MidiDebug:
        applemidi_set_debug_level(3);
        break;
      case MidiWarning:
        applemidi_set_debug_level(1);
        break;
      case MidiError:
        applemidi_set_debug_level(1);
        break;
    }
  }

  static void applemidi_callback_midi_message_received(
      uint8_t port, uint32_t timestamp, uint8_t midi_status,
      uint8_t* remaining_message, size_t len, size_t continued_sysex_pos) {
    MIDI_LOGI("applemidi_callback_midi_message_received: port=%d", port);
    if (_instance) {
      uint8_t message[len + 1];
      message[0] = midi_status;
      memmove(message + 1, remaining_message, len);
      _instance->apple_event_handler.parse(message, len + 1);
    }
  }

  static int32_t applemidi_if_send_udp_datagram(uint8_t* ip_addr, uint16_t port,
                                                uint8_t* tx_data,
                                                size_t tx_len) {
    MIDI_LOGD("applemidi_if_send_udp_datagram: port=%d", port);
    if (_instance) {
      IPAddress address(ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
      UDPClass* p_udp = port == _instance->remote_port
                            ? &(_instance->udpControl)
                            : &(_instance->udpData);
      p_udp->beginPacket(address, port);
      int32_t result = p_udp->write(tx_data, tx_len);
      p_udp->endPacket();
      return result;
    }
    return 0;
  }
};

template <class UDPClass>
AppleMidiServer<UDPClass>* AppleMidiServer<UDPClass>::_instance = nullptr;

}  // namespace midi
#endif