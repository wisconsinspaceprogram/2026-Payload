#pragma once

// Thin wrapper around an Arduino-core HardwareSerial port, so drivers read
// bytes through one small API instead of touching the core directly.
//
// RX buffer size matters: drivers that read once per control cycle need the
// core's RX buffer to hold everything that arrives in between, or bytes are
// dropped. Enlarge it if needed (e.g. Teensy: addMemoryForRead(),
// ESP32: setRxBufferSize() before begin()).

#include <Arduino.h>
#include <cstddef>
#include <cstdint>

class Uart {
public:
    // The serial port (e.g. Serial1) must outlive this object.
    explicit Uart(HardwareSerial& serial);

    // Open the port at `baud`, 8N1.
    void begin(uint32_t baud);

    // Copy up to `max` already-received bytes into `buf` without blocking.
    // Returns the number of bytes copied (0 if none are waiting).
    size_t read(uint8_t* buf, size_t max);

private:
    HardwareSerial& serial_;
};
