#pragma once

// Fake sensor for testing telemetry without hardware attached.
//
// Each dispatch() writes made-up but plausible values to "sim.*" channels:
// a 60 s flight arc that climbs to 3000 m and comes back down, repeating
// forever, with a little noise on every reading. Built only into the
// "dummy" PlatformIO environment (DUMMY_DATA defined), never into flight
// firmware.

#include "blackboard/blackboard.h"

class DummyData {
public:
    // Registers the "sim.*" channels on the blackboard.
    explicit DummyData(Blackboard& bb);

    // Writes this cycle's fake readings to the blackboard.
    void dispatch();

private:
    // Blackboard channels
    float& altitude_m_;           // 0 .. ~3000
    float& vertical_speed_mps_;   // derivative of the altitude arc
    float& temperature_c_;        // 20 C at the ground, standard lapse rate
    float& pressure_hpa_;         // barometric formula from altitude
};
