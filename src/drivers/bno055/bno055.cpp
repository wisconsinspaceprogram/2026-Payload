#include "drivers/bno055/bno055.h"

#include <Arduino.h>

namespace {

constexpr uint8_t kAddr = 0x28;

// Registers
constexpr uint8_t kRegChipId = 0x00;
constexpr uint8_t kRegEulerHeading = 0x1A;
constexpr uint8_t kRegOprMode = 0x3D;

constexpr uint8_t kChipId = 0xA0;
constexpr uint8_t kModeNdof = 0x0C;

// The sensor needs ~650 ms after power-on before it answers on I2C, and
// ~7 ms to leave config mode (datasheet table 0-2 and 3.3).
constexpr uint32_t kBootTimeoutMs = 1000;
constexpr uint32_t kModeSwitchMs = 20;

constexpr float kLsbToDeg = 1.0f / 16.0f;

int16_t readI16(const uint8_t* p) {
    return static_cast<int16_t>(p[0] | (p[1] << 8));
}

}  // namespace

Bno055::Bno055(Blackboard& bb, I2c& i2c)
    : i2c_(i2c),
      yaw_deg_(bb.registerChannel<float>("imu.yaw_deg")),
      pitch_deg_(bb.registerChannel<float>("imu.pitch_deg")),
      roll_deg_(bb.registerChannel<float>("imu.roll_deg")),
      healthy_(bb.registerChannel<bool>("imu.healthy")),
      read_errors_(bb.registerChannel<int>("imu.read_errors")) {}

bool Bno055::init() {
    i2c_.begin();

    uint8_t id = 0;
    uint32_t start = millis();
    while (!i2c_.read(kAddr, kRegChipId, &id, 1) || id != kChipId) {
        if (millis() - start > kBootTimeoutMs) return false;
        delay(10);
    }

    // The sensor powers up in config mode, so this is the only mode change
    if (!i2c_.write(kAddr, kRegOprMode, kModeNdof)) return false;
    delay(kModeSwitchMs);
    return true;
}

void Bno055::dispatch() {
    uint8_t buf[6];
    healthy_ = i2c_.read(kAddr, kRegEulerHeading, buf, sizeof buf);
    if (!healthy_) {
        read_errors_++;
        return;
    }

    yaw_deg_ = readI16(&buf[0]) * kLsbToDeg;
    roll_deg_ = readI16(&buf[2]) * kLsbToDeg;
    pitch_deg_ = readI16(&buf[4]) * kLsbToDeg;
}
