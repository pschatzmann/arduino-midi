#pragma once
#include "MidiCommon.h"
#include "MidiParser.h"
#include "MidiStreamIn.h"
#include "MidiStreamOut.h"
#include "MidiCallbackAction.h"
#include "MidiBleClient.h"		
#include "MidiBleServer.h"		
#include "MidiBleParser.h"
#if TCP_ACTIVE
#include "MidiIpServer.h"
#endif
#if UDP_ACTIVE
#include "MidiUdpServer.h"
#endif
#if APPLE_MIDI_ACTIVE
#include "AppleMidiServer.h"
#endif
#if NAMESPACE_ACTIVE
using namespace midi;
#endif