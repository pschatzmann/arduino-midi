#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "MidiLogger.h"
#include "ConfigMidi.h"

// global actual loggin level for application
MidiLogLevel_t MidiLogLevel= MidiWarning;

// Error level as string
const char* midi_log_msg[] = {"Debug", "Info", "Warning", "Error"};

/// Print log message
void midi_log(MidiLogLevel_t level, const char* fmt ...) {
  if (MidiLogLevel <= level) { // AUDIOKIT_LOG_LEVEL = Debug
    char log_buffer[200];
    strcpy(log_buffer,midi_log_msg[level] );
    strcat(log_buffer,":     ");
    va_list arg;
    va_start(arg, fmt);
    size_t prefix_len = strlen(log_buffer);
    vsnprintf(log_buffer+prefix_len, sizeof(log_buffer)-prefix_len, fmt, arg);
    va_end(arg);
#ifdef LOG_OUT
    LOG_OUT.println(log_buffer);
#else
    puts(log_buffer);
#endif
  }
}
