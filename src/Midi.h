#pragma once
#include "MidiCommon.h"
#include "MidiParser.h"
#include "MidiStreamIn.h"
#include "MidiStreamOut.h"
#include "MidiCallbackAction.h"
#include "MidiBleClient.h"		
#include "MidiBleServer.h"		
#include "MidiBleParser.h"
#include "MidiIpServer.h"
#include "MidiUdpServer.h"
#if APPLE_MIDI_ACTIVE
#include "AppleMidiServer.h"
#endif
using namespace midi;
