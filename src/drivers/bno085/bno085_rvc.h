#pragma once

// Driver for the Adafruit BNO085 breakout in UART-RVC mode.
//
// In RVC ("Robot Vacuum Cleaner") mode the sensor streams 19-byte frames at
// 100 Hz over UART (115200 8N1) containing fused yaw/pitch/roll and
// acceleration. It is transmit-only: the sensor accepts no commands.
//
// Wiring (Adafruit breakout -> MCU):
//   VIN -> 3.3V, GND -> GND, SDA (UART TX in RVC mode) -> a UART RX pin,
//   P0 -> 3.3V (selects RVC mode; sampled at power-up).
//
// Frame layout (all multi-byte fields little-endian):
//   [0..1]   header 0xAA 0xAA
//   [2]      index, increments by 1 per frame (wraps at 255)
//   [3..8]   yaw, pitch, roll   int16, 0.01 deg/LSB
//   [9..14]  accel x, y, z      int16, mg/LSB
//   [15..17] reserved
//   [18]     checksum, low byte of the sum of bytes [2..17]
//
// ~1900 bytes/s arrive, so at a 10 Hz loop the UART RX buffer must hold
// ~190 bytes between dispatch() calls (see uart/uart.h).

#include <cstddef>
#include <cstdint>
#include <optional>

#include "blackboard/blackboard.h"
#include "uart/uart.h"

struct RvcFrame {
    uint8_t index;
    float yaw_deg;    // -180 .. 180
    float pitch_deg;  // -90 .. 90
    float roll_deg;   // -180 .. 180
    float accel_x;    // m/s^2
    float accel_y;
    float accel_z;
};

// Turns the UART byte stream into frames one byte at a time, resyncing on the
// header after garbage or corruption. No heap use and no I/O, so it can be
// fed plain byte arrays in tests.
class RvcParser {
public:
    static constexpr size_t kFrameLen = 19;

    // Feed one received byte. Returns true when this byte completes a frame
    // with a valid checksum; read it with frame().
    bool feed(uint8_t byte);

    // The most recently completed frame.
    const RvcFrame& frame() const { return frame_; }

    // Frames whose header matched but checksum didn't. One or two at startup
    // are normal while the parser syncs to the stream.
    int checksumErrors() const { return checksum_errors_; }

private:
    uint8_t buf_[kFrameLen] = {};
    size_t len_ = 0;
    RvcFrame frame_ = {};
    int checksum_errors_ = 0;
};

class Bno085Rvc {
public:
    // The UART must be wired to the sensor and outlive the driver.
    // Registers the driver's "imu.*" channels on the blackboard.
    Bno085Rvc(Blackboard& bb, Uart& uart);

    // Opens the UART at 115200 baud.
    void init();

    // Drains all bytes received since the last call, publishes the newest
    // frame to the blackboard, and updates the health channels.
    void dispatch();

private:
    // Consecutive dispatches with no new frame before imu.healthy goes false.
    // Frames arrive every 10 ms, so at 10 Hz even one empty dispatch means
    // ~100 ms of silence; 3 tolerates a brief hiccup.
    static constexpr int kMaxMissedDispatches = 3;

    void handleFrame(const RvcFrame& f);

    Uart& uart_;
    RvcParser parser_;
    std::optional<uint8_t> last_index_;
    int missed_dispatches_ = 0;

    // Blackboard channels
    float& yaw_deg_;
    float& pitch_deg_;
    float& roll_deg_;
    float& accel_x_;
    float& accel_y_;
    float& accel_z_;
    bool& healthy_;        // valid frames are arriving
    int& frame_count_;     // valid frames received
    int& dropped_frames_;  // gaps in the frame index (lost or corrupt frames)
    int& checksum_errors_;
};
