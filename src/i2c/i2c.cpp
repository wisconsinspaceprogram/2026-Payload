#include "i2c/i2c.h"

I2c::I2c(TwoWire& wire) : wire_(wire) {}

void I2c::begin() {
    wire_.begin();
}

bool I2c::write(uint8_t addr, uint8_t reg, uint8_t value) {
    wire_.beginTransmission(addr);
    wire_.write(reg);
    wire_.write(value);
    return wire_.endTransmission() == 0;
}

bool I2c::read(uint8_t addr, uint8_t reg, uint8_t* buf, size_t len) {
    wire_.beginTransmission(addr);
    wire_.write(reg);
    // Repeated start: keep the bus between setting the register and reading
    if (wire_.endTransmission(false) != 0) return false;
    if (wire_.requestFrom(addr, static_cast<uint8_t>(len)) != len) return false;
    for (size_t i = 0; i < len; i++) {
        buf[i] = static_cast<uint8_t>(wire_.read());
    }
    return true;
}
