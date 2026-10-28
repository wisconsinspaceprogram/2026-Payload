#include "spi/spi.h"

namespace {

// Well under the 10 MHz limit of the parts we use, so wiring is forgiving
const SPISettings kSettings(1000000, MSBFIRST, SPI_MODE0);

constexpr uint8_t kWriteBit = 0x80;

}  // namespace

Spi::Spi(SPIClass& spi, uint8_t cs_pin) : spi_(spi), cs_pin_(cs_pin) {}

void Spi::begin() {
    pinMode(cs_pin_, OUTPUT);
    digitalWrite(cs_pin_, HIGH);
    spi_.begin();
}

void Spi::write(uint8_t reg, uint8_t value) {
    write(reg, &value, 1);
}

void Spi::write(uint8_t reg, const uint8_t* buf, size_t len) {
    select();
    spi_.transfer(reg | kWriteBit);
    for (size_t i = 0; i < len; i++) {
        spi_.transfer(buf[i]);
    }
    deselect();
}

uint8_t Spi::read(uint8_t reg) {
    select();
    spi_.transfer(reg & ~kWriteBit);
    uint8_t value = spi_.transfer(0x00);
    deselect();
    return value;
}

void Spi::select() {
    spi_.beginTransaction(kSettings);
    digitalWrite(cs_pin_, LOW);
}

void Spi::deselect() {
    digitalWrite(cs_pin_, HIGH);
    spi_.endTransaction();
}
