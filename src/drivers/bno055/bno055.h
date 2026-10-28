#pragma once

// Driver for the Adafruit BNO055 breakout over I2C.
//
// The sensor runs its own fusion in NDOF mode (accel + gyro + mag) and keeps
// the latest Euler angles in registers, so each dispatch() is one 6-byte read.
//
// Wiring (Adafruit breakout -> Teensy), via STEMMA QT or the header pins:
//   VIN -> 3.3V, GND -> GND, SDA -> SDA (pin 18), SCL -> SCL (pin 19).
//   ADR left floating selects address 0x28.
//
// Euler registers (little-endian int16, 1/16 deg/LSB in the default units):
//   0x1A heading, 0x1C roll, 0x1E pitch

#include <cstdint>

#include "blackboard/blackboard.h"
#include "i2c/i2c.h"

class Bno055 {
public:
    // The bus must be wired to the sensor and outlive the driver.
    // Registers the driver's "imu.*" channels on the blackboard.
    Bno055(Blackboard& bb, I2c& i2c);

    // Starts the bus, waits for the sensor to boot, and switches it to NDOF
    // fusion mode. Returns false if the sensor didn't respond.
    bool init();

    // Reads the latest Euler angles into the blackboard and updates the
    // health channels.
    void dispatch();

private:
    I2c& i2c_;

    // Blackboard channels
    float& yaw_deg_;    // 0 .. 360, the BNO055's "heading"
    float& pitch_deg_;  // -180 .. 180
    float& roll_deg_;   // -90 .. 90
    bool& healthy_;     // the last read succeeded
    int& read_errors_;  // failed reads
};
