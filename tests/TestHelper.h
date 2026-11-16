#pragma once
// Minimal dependency free test helpers
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "MidiAction.h"

/// Redirects stdout (e.g. the log output) into a string
class StdoutCapture {
 public:
  StdoutCapture() {
    fflush(stdout);
    tmp = tmpfile();
    saved = dup(1);
    dup2(fileno(tmp), 1);
  }
  ~StdoutCapture() { stop(); }

  /// Restores stdout and returns everything written in the meantime
  std::string stop() {
    if (tmp == nullptr) return result;
    fflush(stdout);
    dup2(saved, 1);
    close(saved);
    rewind(tmp);
    char buf[256];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), tmp)) > 0) result.append(buf, n);
    fclose(tmp);
    tmp = nullptr;
    return result;
  }

 private:
  FILE* tmp = nullptr;
  int saved = -1;
  std::string result;
};

static int test_failures = 0;
static int test_count = 0;

#define CHECK(cond)                                                     \
  do {                                                                  \
    test_count++;                                                       \
    if (!(cond)) {                                                      \
      test_failures++;                                                  \
      printf("FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond);          \
    }                                                                   \
  } while (0)

#define CHECK_EQ(a, b)                                                  \
  do {                                                                  \
    test_count++;                                                       \
    if (!((a) == (b))) {                                                \
      test_failures++;                                                  \
      printf("FAILED %s:%d: %s == %s\n", __FILE__, __LINE__, #a, #b);   \
    }                                                                   \
  } while (0)

/// Prints the result and returns the exit code for main()
inline int testResult(const char* name) {
  printf("%s: %d checks, %d failures\n", name, test_count, test_failures);
  return test_failures == 0 ? 0 : 1;
}

/// MidiAction which records all events as strings
class RecordingAction : public midi::MidiAction {
 public:
  std::vector<std::string> events;

  void onNoteOn(uint8_t ch, uint8_t note, uint8_t vel) override {
    add("on", ch, note, vel);
  }
  void onNoteOff(uint8_t ch, uint8_t note, uint8_t vel) override {
    add("off", ch, note, vel);
  }
  void onControlChange(uint8_t ch, uint8_t cc, uint8_t value) override {
    add("cc", ch, cc, value);
  }
  void onPitchBend(uint8_t ch, uint8_t value) override {
    add("pb", ch, value);
  }
  void onProgramChange(uint8_t ch, uint8_t program) override {
    add("pc", ch, program);
  }
  void onChannelPressure(uint8_t ch, uint8_t pressure) override {
    add("cp", ch, pressure);
  }
  void onPolyPressure(uint8_t ch, uint8_t note, uint8_t pressure) override {
    add("pp", ch, note, pressure);
  }

  /// Returns the recorded events and clears them
  std::vector<std::string> take() {
    std::vector<std::string> result;
    result.swap(events);
    return result;
  }

 protected:
  void add(const char* name, int a, int b = -1, int c = -1) {
    std::string s = name;
    for (int v : {a, b, c}) {
      if (v >= 0) s += " " + std::to_string(v);
    }
    events.push_back(s);
  }
};

/// Records the full 14 bit pitch bend
class RecordingAction14 : public RecordingAction {
 public:
  void onPitchBend14(uint8_t ch, uint16_t value) override {
    add("pb14", ch, value);
  }
};

typedef std::vector<std::string> Events;
