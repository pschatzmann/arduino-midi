#pragma once

enum MidiLogLevel_t {MidiDebug, MidiInfo, MidiWarning, MidiError};

extern MidiLogLevel_t MidiLogLevel;

void midi_log(MidiLogLevel_t level, const char* fmr,...);

#define MIDI_LOGD(fmt,...) midi_log(MidiDebug, fmt, ##__VA_ARGS__)
#define MIDI_LOGI(fmt,...) midi_log(MidiInfo, fmt, ##__VA_ARGS__)
#define MIDI_LOGW(fmt,...) midi_log(MidiWarning, fmt, ##__VA_ARGS__)
#define MIDI_LOGE(fmt,...) midi_log(MidiError, fmt, ##__VA_ARGS__)


