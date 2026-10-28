#pragma once

// Thin wrapper around an Arduino-core SPIClass bus plus one chip-select pin,
// so drivers access registers through one small API instead of touching the
// core directly. Uses the common register convention: the address's MSB is
// set for writes and clear for reads.

#include <Arduino.h>
#include <SPI.h>
#include <cstddef>
#include <cstdint>

class Spi {
public:
    // The bus (e.g. SPI) must outlive this object.
    Spi(SPIClass& spi, uint8_t cs_pin);

    // Start the bus and deselect the device.
    void begin();

    // Write one byte to register `reg`.
    void write(uint8_t reg, uint8_t value);

    // Write `len` bytes to register `reg` in one burst (e.g. a FIFO).
    void write(uint8_t reg, const uint8_t* buf, size_t len);

    // Read one byte from register `reg`. SPI has no acknowledge, so a
    // missing device reads as 0x00 or 0xFF rather than failing.
    uint8_t read(uint8_t reg);

private:
    void select();
    void deselect();

    SPIClass& spi_;
    uint8_t cs_pin_;
};
