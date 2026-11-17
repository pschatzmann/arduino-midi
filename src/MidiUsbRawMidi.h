#pragma once
#include "ConfigMidi.h"

#if MIDI_ACTIVE && defined(__linux__)

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <vector>

#include "MidiUsbBase.h"

namespace midi {

/***************************************************/
/*! \class MidiUsbRawMidi
    \brief USB MIDI on a Linux desktop (or a Raspberry Pi) using the ALSA
    raw MIDI device files (/dev/snd/midiC<card>D<device>). No additional
    library is needed.

    This covers both USB roles:
    - USB host: a USB MIDI keyboard/controller plugged into the computer
    - USB device: the computer acts as USB MIDI gadget (e.g. a Raspberry Pi
      Zero with the g_midi / f_midi gadget driver)

    In begin() you can indicate the device file, a part of the ALSA card name
    (e.g. "Keystation") or nothing to use the first available device. If the
    device is unplugged, loop() reconnects automatically when it is plugged
    in again.

    by Phil Schatzmann
*/
/***************************************************/
class MidiUsbRawMidi : public MidiUsbBase {
 public:
  MidiUsbRawMidi() = default;
  /// Constructor with the MidiAction and an optional channel filter
  MidiUsbRawMidi(MidiAction* action, int channelFilter = -1)
      : MidiUsbBase(action, channelFilter) {}
  ~MidiUsbRawMidi() { end(); }

  /// Opens the indicated device: a path (e.g. "/dev/snd/midiC1D0"), a part
  /// of the card name (e.g. "Keystation") or nullptr for the first device
  bool begin(const char* device = nullptr) {
    selector = device == nullptr ? "" : device;
    is_active = true;
    bool ok = open();
    if (!ok) MIDI_LOGE("MidiUsb: no MIDI device found for '%s'", selector.c_str());
    return ok;
  }

  /// Starts with the indicated MidiAction
  bool begin(MidiAction& action, const char* device = nullptr) {
    setMidiAction(action);
    return begin(device);
  }

  /// Closes the device
  void end() {
    is_active = false;
    close();
  }

  /// Processes the received messages and reconnects a device that has been
  /// unplugged: call it in the loop
  bool loop() override {
    if (fd < 0) {
      reconnect();
      if (fd < 0) return false;
    }
    bool processed = false;
    uint8_t buffer[64];
    while (true) {
      ssize_t len = ::read(fd, buffer, sizeof(buffer));
      if (len > 0) {
        MidiParser* parser = activeParser();
        if (parser != nullptr) parser->parse(buffer, (int)len);
        processed = true;
        continue;
      }
      if (len < 0 && errno != EAGAIN && errno != EWOULDBLOCK &&
          errno != EINTR) {
        // e.g. ENODEV: the device was unplugged
        MIDI_LOGW("MidiUsb: %s disconnected (%s)", path.c_str(),
                  strerror(errno));
        close();
        setConnectionStatus(Disconnected);
      }
      break;
    }
    return processed;
  }

  /// Writes the packet as raw MIDI bytes
  bool writePacket(const uint8_t packet[4]) override {
    uint8_t bytes[3];
    int len = MidiUsbPacket::toBytes(packet, bytes);
    return writeBytes(bytes, len);
  }

  /// Not supported: the raw MIDI device provides a byte stream (use loop())
  bool readPacket(uint8_t packet[4]) override { return false; }

  /// Writes raw MIDI bytes (e.g. system exclusive messages)
  bool writeBytes(const uint8_t* data, int len) {
    if (fd < 0 || len <= 0) return false;
    int pos = 0;
    int retry = 0;
    while (pos < len) {
      ssize_t written = ::write(fd, data + pos, len - pos);
      if (written > 0) {
        pos += (int)written;
      } else if (written < 0 && (errno == EAGAIN || errno == EINTR) &&
                 retry++ < 100) {
        usleep(1000);  // output buffer full: wait a bit
      } else {
        MIDI_LOGE("MidiUsb: write to %s failed", path.c_str());
        return false;
      }
    }
    return true;
  }

  /// Provides the path of the opened device file (empty if not open)
  const char* devicePath() { return fd < 0 ? "" : path.c_str(); }

  /// Lists the available raw MIDI device files (e.g. /dev/snd/midiC1D0)
  static std::vector<std::string> devices() {
    std::vector<std::string> result;
    DIR* dir = opendir("/dev/snd");
    if (dir == nullptr) return result;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      if (strncmp(entry->d_name, "midiC", 5) == 0) {
        result.push_back(std::string("/dev/snd/") + entry->d_name);
      }
    }
    closedir(dir);
    std::sort(result.begin(), result.end());
    return result;
  }

  /// Provides the ALSA card name of a raw MIDI device file
  static std::string cardName(const std::string& devicePath) {
    int card = -1, dev = -1;
    const char* name = strrchr(devicePath.c_str(), '/');
    name = name == nullptr ? devicePath.c_str() : name + 1;
    if (sscanf(name, "midiC%dD%d", &card, &dev) != 2) return "";
    char file[64];
    snprintf(file, sizeof(file), "/proc/asound/card%d/id", card);
    std::string result;
    FILE* f = fopen(file, "r");
    if (f != nullptr) {
      char line[80];
      if (fgets(line, sizeof(line), f) != nullptr) {
        result = line;
        while (!result.empty() &&
               (result.back() == '\n' || result.back() == ' ')) {
          result.pop_back();
        }
      }
      fclose(f);
    }
    return result;
  }

 protected:
  int fd = -1;
  bool is_active = false;
  std::string selector;
  std::string path;
  time_t last_reconnect = 0;

  /// Determines the device file for the selector
  std::string findDevice() {
    // explicit path
    if (!selector.empty() && selector[0] == '/') return selector;
    for (auto& dev : devices()) {
      if (selector.empty()) return dev;
      std::string card = cardName(dev);
      if (strcasestr(card.c_str(), selector.c_str()) != nullptr) return dev;
    }
    return "";
  }

  bool open() {
    close();
    path = findDevice();
    if (path.empty()) return false;
    fd = ::open(path.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
      MIDI_LOGE("MidiUsb: could not open %s: %s", path.c_str(),
                strerror(errno));
      return false;
    }
    MIDI_LOGI("MidiUsb: opened %s (%s)", path.c_str(),
              cardName(path).c_str());
    setConnectionStatus(Connected);
    return true;
  }

  void close() {
    if (fd >= 0) {
      ::close(fd);
      fd = -1;
    }
  }

  /// Tries to reopen the device at most once per second
  void reconnect() {
    if (!is_active) return;
    time_t now = time(nullptr);
    if (now == last_reconnect) return;
    last_reconnect = now;
    open();
  }
};

}  // namespace midi

#endif
