#include "drivers/bno085/bno085_rvc.h"

#include <cstring>

namespace {

constexpr uint8_t kHeaderByte = 0xAA;
constexpr uint32_t kBaud = 115200;
constexpr float kCentiDegToDeg = 0.01f;
constexpr float kMgToMs2 = 9.80665f / 1000.0f;

int16_t readI16(const uint8_t* p) {
    return static_cast<int16_t>(p[0] | (p[1] << 8));
}

}  // namespace

bool RvcParser::feed(uint8_t byte) {
    // Still hunting for the two header bytes
    if (len_ < 2) {
        if (byte == kHeaderByte) {
            buf_[len_++] = byte;
        } else {
            len_ = 0;
        }
        return false;
    }

    buf_[len_++] = byte;
    if (len_ < kFrameLen) return false;
    len_ = 0;

    uint8_t sum = 0;
    for (size_t i = 2; i < kFrameLen - 1; i++) sum += buf_[i];
    if (sum != buf_[kFrameLen - 1]) {
        checksum_errors_++;
        // We may have synced on a stray 0xAA, with the real header later in
        // this window; rescan everything after the first byte. Only 18 bytes
        // are re-fed, so this can't complete a frame or recurse further.
        uint8_t rest[kFrameLen - 1];
        memcpy(rest, &buf_[1], sizeof rest);
        for (uint8_t b : rest) feed(b);
        return false;
    }

    frame_.index = buf_[2];
    frame_.yaw_deg = readI16(&buf_[3]) * kCentiDegToDeg;
    frame_.pitch_deg = readI16(&buf_[5]) * kCentiDegToDeg;
    frame_.roll_deg = readI16(&buf_[7]) * kCentiDegToDeg;
    frame_.accel_x = readI16(&buf_[9]) * kMgToMs2;
    frame_.accel_y = readI16(&buf_[11]) * kMgToMs2;
    frame_.accel_z = readI16(&buf_[13]) * kMgToMs2;
    return true;
}

Bno085Rvc::Bno085Rvc(Blackboard& bb, Uart& uart)
    : uart_(uart),
      yaw_deg_(bb.registerChannel<float>("imu.yaw_deg")),
      pitch_deg_(bb.registerChannel<float>("imu.pitch_deg")),
      roll_deg_(bb.registerChannel<float>("imu.roll_deg")),
      accel_x_(bb.registerChannel<float>("imu.accel_x")),
      accel_y_(bb.registerChannel<float>("imu.accel_y")),
      accel_z_(bb.registerChannel<float>("imu.accel_z")),
      healthy_(bb.registerChannel<bool>("imu.healthy")),
      frame_count_(bb.registerChannel<int>("imu.frame_count")),
      dropped_frames_(bb.registerChannel<int>("imu.dropped_frames")),
      checksum_errors_(bb.registerChannel<int>("imu.checksum_errors")) {}

void Bno085Rvc::init() {
    uart_.begin(kBaud);
}

void Bno085Rvc::dispatch() {
    // Several frames arrive per cycle; each overwrites the last, so the
    // blackboard ends up holding the newest one.
    bool got_frame = false;
    uint8_t buf[64];
    size_t n;
    while ((n = uart_.read(buf, sizeof buf)) > 0) {
        for (size_t i = 0; i < n; i++) {
            if (parser_.feed(buf[i])) {
                handleFrame(parser_.frame());
                got_frame = true;
            }
        }
    }

    missed_dispatches_ = got_frame ? 0 : missed_dispatches_ + 1;
    healthy_ = frame_count_ > 0 && missed_dispatches_ < kMaxMissedDispatches;
    checksum_errors_ = parser_.checksumErrors();
}

void Bno085Rvc::handleFrame(const RvcFrame& f) {
    if (last_index_) {
        // uint8_t arithmetic handles the 255 -> 0 wrap
        dropped_frames_ += static_cast<uint8_t>(f.index - *last_index_ - 1);
    }
    last_index_ = f.index;
    frame_count_++;

    yaw_deg_ = f.yaw_deg;
    pitch_deg_ = f.pitch_deg;
    roll_deg_ = f.roll_deg;
    accel_x_ = f.accel_x;
    accel_y_ = f.accel_y;
    accel_z_ = f.accel_z;
}
