/*
 * Apple MIDI Driver
 *
 * See README.md for usage hints
 *
 * =============================================================================
 *
 * MIT License
 *
 * Copyright (c) 2020 Thorsten Klose (tk@midibox.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * =============================================================================
 */

#ifndef _APPLEMIDI_H
#define _APPLEMIDI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#ifndef htons
#define htons(x) ( ((x)<< 8 & 0xFF00) | \
                   ((x)>> 8 & 0x00FF) )
#define ntohs(x) htons(x)
#endif

#ifndef htonl
#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#define ntohl(x) htonl(x)
#endif


#ifndef APPLEMIDI_DEFAULT_DEBUG_LEVEL
#define APPLEMIDI_DEFAULT_DEBUG_LEVEL 1
#endif

#ifndef APPLEMIDI_MAX_PEERS
#define APPLEMIDI_MAX_PEERS 5 // including myself
#endif

#ifndef APPLEMIDI_DEFAULT_PORT
#define APPLEMIDI_DEFAULT_PORT 5004
#endif

#ifndef APPLEMIDI_MAX_NAME_LEN
#define APPLEMIDI_MAX_NAME_LEN 64
#endif

#ifndef APPLEMIDI_MY_DEFAULT_NAME
#define APPLEMIDI_MY_DEFAULT_NAME "MIDIbox"
#endif

#ifndef APPLEMIDI_LOG_TAG
#define APPLEMIDI_LOG_TAG "[APPLEMIDI] "
#endif

#ifndef APPLEMIDI_BITRATE_RECEIVE_LIMIT
// TODO: doesn't really work with MacOS, therefore disabled it
//#define APPLEMIDI_BITRATE_RECEIVE_LIMIT 1000
#endif

#ifndef APPLEMIDI_OUTBUFFER_SIZE
#define APPLEMIDI_OUTBUFFER_SIZE 512
#endif

#ifndef APPLEMIDI_OUTBUFFER_FLUSH_MS
#define APPLEMIDI_OUTBUFFER_FLUSH_MS 1
#endif

// if master: how often do we want to synchronize?
#ifndef APPLEMIDI_MASTER_START_SYNC_MS
#define APPLEMIDI_MASTER_START_SYNC_MS 100
#endif

#ifndef APPLEMIDI_MASTER_REGULAR_SYNC_MS
#define APPLEMIDI_MASTER_REGULAR_SYNC_MS 20*1000
#endif


typedef enum {
  APPLEMIDI_CONNECTION_STATE_SLAVE = 0,
  APPLEMIDI_CONNECTION_STATE_MASTER_CONNECT_CTRL,
  APPLEMIDI_CONNECTION_STATE_MASTER_CONNECT_DATA,
  APPLEMIDI_CONNECTION_STATE_MASTER_CONNECTED,
} applemidi_connection_state_t;

//! contains information about the peers
//! Peer 0 is always myself, peer 1..APPLEMIDI_MAX_NAME_LEN-1 are remote connections
typedef struct {
  applemidi_connection_state_t connection_state;
  uint32_t  connection_sync_done_timestamp;
  uint8_t   connection_sync_ctr;

  uint32_t ssrc;
  uint32_t token;
  char name[APPLEMIDI_MAX_NAME_LEN];
  uint8_t  ip_addr[16]; // for IPv4 and IPv6
  uint16_t control_port; // if 0: no connection, if >0: peer is active
  uint16_t data_port; // if 0: no connection, if >0: peer is active
  uint8_t  applemidi_port; // internal port number
  uint16_t seq_nr;
  uint32_t continued_sysex_pos;

  // we buffer outgoing MIDI messages for 2 mS - this should avoid that multiple packets have to be queued for small messages
  uint32_t outbuffer_timestamp_last_flush;
  uint32_t outbuffer[APPLEMIDI_OUTBUFFER_SIZE/4];
  uint16_t outbuffer_len;

  // statistics
  uint32_t packets_sent;
  uint32_t packets_received;
  uint32_t packets_loss;
} applemidi_peer_t;


/**
 * @brief Initializes the Apple MIDI Driver
 *
 * @param  callback_midi_message_received References the callback function which is called whenever a new MIDI message has been received.
 *         API see applemidi_receive_packet_callback_for_debugging
 *         Specify NULL if no callback required in your application.
 * @param  callback_send_packet References the callback function which is called whenever a UDP datagram should be sent.
 *         API see applemidi_send_udp_datagram_for_debugging
 *         Specify NULL if no callback required in your application (very unlikely... ;-)
 */
extern int32_t applemidi_init(void *callback_midi_message_received, void *_callback_send_udp_datagram);

/**
 * @brief Returns information about a peer
 *
 */
extern applemidi_peer_t *applemidi_peer_get_info(uint8_t applemidi_port);

/**
 * @brief Returns free applemidi_port (1..APPLEMIDI_MAX_PEERS-1), or < 0 if all ports allocated
 *
 */
extern int32_t applemidi_search_free_port(void);

/**
 * @brief Sets the verbosity level
 *
 */
extern int32_t applemidi_set_debug_level(uint8_t verbosity);

/**
 * @brief Returns the verbosity level
 *
 */
extern int32_t applemidi_get_debug_level(void);

/**
 * @brief Sends a Apple MIDI packet
 *
 * @param  applemidi_port currently always 0 expected (we might support multiple ports in future)
 * @param  stream       output stream
 * @param  len          output stream length
 *
 * @return < 0 on errors
 *
 */
extern int32_t applemidi_send_message(uint8_t applemidi_port, uint8_t *stream, size_t len);

/**
 * @brief This function should be called each mS to handle the output buffers and synchronization
 *
 * @return < 0 on errors
 */
extern void applemidi_tick(void);

/**
 * @brief Flush Output Buffer (normally done by applemidi_tick each 5 mS)
 *
 * @param  applemidi_port currently always 0 expected (we might support multiple ports in future)
 *
 * @return < 0 on errors
 *
 */
extern int32_t applemidi_outbuffer_flush(uint8_t applemidi_port);

/**
 * @brief A dummy callback which demonstrates the usage.
 *        It will just print out incoming MIDI messages on the terminal.
 *        You might want to implement your own for doing something more useful!
 * @param  applemidi_port currently always 0 expected (we might support multiple ports in future)
 * @param  timestamp    the timestamp
 * @param  midi_status  the MIDI status byte (first byte of a MIDI message)
 * @param  remaining_message the remaining bytes
 * @param  len          size of the remaining message
 * @param  continued_sysex_pos in case the next part of a SysEx stream has been received, this variable is >0 and passes the position of the continued sysex stream
 *
 * @return < 0 on errors
 */
extern void applemidi_receive_packet_callback_for_debugging(uint8_t applemidi_port, uint32_t timestamp, uint8_t midi_status, uint8_t *remaining_message, size_t len, size_t continued_sysex_pos);

/**
 * @brief A dummy callback which demonstrates the usage.
 *        It will just print out the UDP datagram which should be sent on the terminal.
 *        To get Apple MIDI communication working, this callback has to be implemented in your application.
 *
 * @param  ip_addr pointer to the IP address (4 or 16 bytes, depending in IPv4 and IPv6)
 * @param  port port number
 * @param  tx_data data which should be sent
 * @param  tx_len packet size
 *
 * @return < 0 on errors
 */
extern int32_t applemidi_callback_send_udp_datagram_for_debugging(uint8_t *ip_addr, uint16_t port, uint8_t *tx_data, size_t tx_len);

/**
 * @brief Parses an incoming UDP Datagram for RTP and Apple MIDI messages
 *
 * @return < 0 on errors
 */
extern int32_t applemidi_parse_udp_datagram(uint8_t *ip_addr, uint16_t port, uint8_t *rx_data, size_t rx_len, uint8_t is_dataport);


/**
 * @brief Invites a peer for the given applemidi_port
 *
 */
extern int32_t applemidi_start_session(uint8_t applemidi_port, uint8_t *ip_addr, uint16_t control_port);

/**
 * @brief Terminates a session for the given applemidi_port
 *
 */
extern int32_t applemidi_terminate_session(uint8_t applemidi_port);


#ifdef __cplusplus
}
#endif

#endif /* _APPLEMIDI_H */