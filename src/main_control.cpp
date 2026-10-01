#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <thread>
#include "blackboard/blackboard.h"

// 10 Hz means 100 milliseconds per cycle
const std::chrono::milliseconds interval(100); 

int main() {
    // initialize blackboard
    Blackboard bb;
    // Unix time the current cycle started (for logging/telemetry only)
    TimePoint& clock_time = bb.registerChannel<TimePoint>("clock_time", std::chrono::system_clock::now());
    int& cycle_count = bb.registerChannel<int>("cycle_count", 0);

    // run init() of all drivers

    // Schedule on steady_clock, which never jumps; system_clock can jump
    // when NTP/GPS sets the time, which would stall or burst the loop.
    auto next_cycle = std::chrono::steady_clock::now();

    while (true) {
        clock_time = std::chrono::system_clock::now();
        cycle_count++;

        // Unix time in seconds, to the millisecond
        std::cout << "clock_time: " << std::fixed << std::setprecision(3)
                  << std::chrono::duration<double>(clock_time.time_since_epoch()).count() << std::endl;
        std::cout << "cycle_count: " << cycle_count << std::endl;

        // simpleRTK2B dispatch()
        // BNO085 dispatch()
        // Radio dispatch()

        next_cycle += interval;
        std::this_thread::sleep_until(next_cycle);
    }

    return 0;
}
