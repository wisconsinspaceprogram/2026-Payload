#include <Arduino.h>
#include <cstdint>
#include "blackboard/blackboard.h"
#include "drivers/bno055/bno055.h"
#include "drivers/rfm95/rfm95.h"
#include "i2c/i2c.h"
#include "spi/spi.h"

// The Teensy core provides main(): it calls setup() once, then loop() forever.
// Everything below is global so both can reach it. Constructors only register
// blackboard channels; hardware is touched in setup().

// 10 Hz means 100 milliseconds per cycle
constexpr uint32_t kIntervalMs = 100;

// How long setup() waits for a serial monitor before running without one
constexpr uint32_t kSerialWaitMs = 2000;

// initialize blackboard
Blackboard bb;
// Unix time in seconds from the Teensy's RTC (for logging/telemetry only).
// Uploading from Teensyduino sets the RTC to the PC's clock; it keeps
// time across resets only with a coin cell on VBAT.
uint32_t& clock_time = bb.registerChannel<uint32_t>("clock_time");
// Milliseconds since boot when the current cycle started
uint32_t& uptime_ms = bb.registerChannel<uint32_t>("uptime_ms");
int& cycle_count = bb.registerChannel<int>("cycle_count", 0);

I2c i2c(Wire);
Bno055 bno055(bb, i2c);
Spi radio_spi(SPI, 10);
Rfm95 rfm95(bb, radio_spi);

// Schedule on millis(), which never jumps; the RTC can jump when GPS
// sets the time, which would stall or burst the loop.
uint32_t next_cycle;

void setup() {
    // USB serial ignores the baud rate. Wait briefly so init messages aren't
    // lost, but never block flight on a missing monitor.
    Serial.begin(115200);
    while (!Serial && millis() < kSerialWaitMs) {
    }

    // run init() of all drivers
    if (!bno055.init()) {
        Serial.println("BNO055 init failed");
    }
    if (!rfm95.init()) {
        Serial.println("RFM95 init failed");
    }

    next_cycle = millis();
}

void loop() {
    // Return early rather than busy-wait, so the core's yield() keeps running
    // between calls. Signed difference so this keeps working when millis()
    // wraps (~49 days).
    if (static_cast<int32_t>(millis() - next_cycle) < 0) return;
    next_cycle += kIntervalMs;

    clock_time = Teensy3Clock.get();
    uptime_ms = millis();
    cycle_count++;

    Serial.print("clock_time: ");
    Serial.println(clock_time);
    Serial.print("uptime_ms: ");
    Serial.println(uptime_ms);
    Serial.print("cycle_count: ");
    Serial.println(cycle_count);

    // simpleRTK2B dispatch()
    bno055.dispatch();
    // Radio last, so the packet carries this cycle's values
    rfm95.dispatch();
}
