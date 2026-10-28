#pragma once

// Thin wrapper around an Arduino-core TwoWire bus, so drivers access
// registers through one small API instead of touching the core directly.

#include <Arduino.h>
#include <Wire.h>
#include <cstddef>
#include <cstdint>

class I2c {
public:
    // The bus (e.g. Wire) must outlive this object.
    explicit I2c(TwoWire& wire);

    // Start the bus as controller at the core's default clock (100 kHz).
    void begin();

    // Write one byte to register `reg` of the device at `addr`.
    // Returns false if the device didn't acknowledge.
    bool write(uint8_t addr, uint8_t reg, uint8_t value);

    // Read `len` consecutive bytes starting at register `reg` into `buf`.
    // Returns false if the device didn't acknowledge or sent fewer bytes.
    bool read(uint8_t addr, uint8_t reg, uint8_t* buf, size_t len);

private:
    TwoWire& wire_;
};
