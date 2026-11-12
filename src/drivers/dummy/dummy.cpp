#include "drivers/dummy/dummy.h"

#include <Arduino.h>
#include <cmath>

namespace {

constexpr float kFlightPeriodS = 60.0f;
constexpr float kApogeeM = 3000.0f;

constexpr float kGroundTempC = 20.0f;
constexpr float kLapseRateCPerM = 0.0065f;
constexpr float kSeaLevelHpa = 1013.25f;

// Uniform noise in [-amplitude, amplitude]
float noise(float amplitude) {
    return amplitude * (static_cast<float>(random(-1000, 1001)) / 1000.0f);
}

}  // namespace

DummyData::DummyData(Blackboard& bb)
    : altitude_m_(bb.registerChannel<float>("sim.altitude_m")),
      vertical_speed_mps_(bb.registerChannel<float>("sim.vertical_speed_mps")),
      temperature_c_(bb.registerChannel<float>("sim.temperature_c")),
      pressure_hpa_(bb.registerChannel<float>("sim.pressure_hpa")) {}

void DummyData::dispatch() {
    // Position in the current flight, 0 .. 1
    float phase = fmodf(millis() / 1000.0f, kFlightPeriodS) / kFlightPeriodS;

    // Half a sine wave: up to apogee at phase 0.5, back down at 1
    float altitude = kApogeeM * sinf(PI * phase);
    float speed = kApogeeM * PI / kFlightPeriodS * cosf(PI * phase);

    altitude_m_ = altitude + noise(2.0f);
    vertical_speed_mps_ = speed + noise(0.5f);
    temperature_c_ = kGroundTempC - kLapseRateCPerM * altitude + noise(0.1f);
    pressure_hpa_ = kSeaLevelHpa * powf(1.0f - 2.25577e-5f * altitude, 5.25588f) + noise(0.05f);
}
