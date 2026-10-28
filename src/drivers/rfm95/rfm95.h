#pragma once

// Driver for the Adafruit RFM95W LoRa breakout (915 MHz) over SPI, transmit
// only.
//
// Each dispatch() sends every blackboard channel as one text packet, e.g.
//   clock_time=1793200000.123,cycle_count=12,imu.yaw_deg=3.100,...
// A packet takes a few hundred ms on air, longer than one loop cycle, so
// dispatch() skips cycles while the previous packet is still sending.
// Channels that don't fit in the 255-byte LoRa limit are left off the end.
//
// Radio settings are the chip's defaults except where noted: 915 MHz,
// SF7, 125 kHz bandwidth, coding rate 4/5, explicit header, CRC on, 17 dBm.
// The ground receiver must use the same settings.
//
// Wiring (Adafruit breakout -> Teensy):
//   VIN -> 3.3V, GND -> GND, SCK -> 13, MISO -> 12, MOSI -> 11,
//   CS -> 10, RST -> 9. G0 (DIO0) is not used.
// Never transmit without an antenna on the ANT pad; it can damage the radio.

#include <cstdint>

#include "blackboard/blackboard.h"
#include "spi/spi.h"

class Rfm95 {
public:
    // The bus must be wired to the radio and outlive the driver. The
    // blackboard is read on every dispatch(), so it must outlive it too.
    // Registers the driver's "radio.*" channels on the blackboard.
    Rfm95(Blackboard& bb, Spi& spi);

    // Resets the radio, starts the bus, and configures it for LoRa at
    // 915 MHz. Returns false if the radio didn't respond.
    bool init();

    // Sends the blackboard as one packet unless the previous packet is still
    // on air, and updates the health channels. Call it last in the cycle so
    // the packet carries this cycle's values.
    void dispatch();

private:
    const Blackboard& bb_;
    Spi& spi_;

    // Blackboard channels
    bool& healthy_;      // the radio answered in LoRa mode at the last dispatch
    int& packets_sent_;  // packets handed to the radio
};
