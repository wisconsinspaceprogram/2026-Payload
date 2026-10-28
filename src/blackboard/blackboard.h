#pragma once

// Blackboard: a central store of named telemetry channels.
// Not thread-safe; use only from the main control loop.

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <variant>

// uint32_t holds times: Unix seconds from the RTC or milliseconds from millis()
using Value = std::variant<bool, int, uint32_t, float, double, std::string>;

struct Channel {
    std::string name;
    Value value;
};

class Blackboard {
public:
    // Add a new channel of type T and return a reference to its value, so it
    // can be read/written like a plain variable with no name lookup. The
    // initial value is optional (defaults to 0 / false / ""). Halts with an
    // error over Serial if the name is empty or already registered (the
    // Teensy builds without exceptions, and this is always a coding mistake).
    // Example:
    //   float& roll = bb.registerChannel<float>("imu.roll");
    //   roll = 12.5f;
    //
    // The reference stays valid for the Blackboard's lifetime: channels_ never
    // moves existing elements, and a channel's type never changes (write()
    // rejects mismatched types), so the variant always holds this same T.
    template <typename T>
    T& registerChannel(const std::string& name, const T& initial = T{}) {
        if (name.empty()) halt("Blackboard: empty channel name");
        if (index_.count(name)) halt("Blackboard: duplicate channel '" + name + "'");
        index_[name] = channels_.size();
        channels_.push_back({name, Value{initial}});
        return std::get<T>(channels_.back().value);
    }

    // Overwrite a channel's value. Returns false if the channel doesn't exist
    // or the value's type doesn't match the channel's type.
    bool write(const std::string& name, const Value& value);

    // Read a channel into `out`. Returns false if the channel doesn't exist
    // or T doesn't match the channel's type.
    template <typename T>
    bool read(const std::string& name, T& out) const {
        auto it = index_.find(name);
        if (it == index_.end()) return false;
        const T* v = std::get_if<T>(&channels_[it->second].value);
        if (!v) return false;
        out = *v;
        return true;
    }

    // All channels, in the order they were registered.
    const std::deque<Channel>& channels() const { return channels_; }

private:
    // Print `msg` over Serial once a second, forever, so it is seen whenever
    // a serial monitor connects.
    [[noreturn]] static void halt(const std::string& msg);

    // deque (not vector) so adding channels never moves existing ones,
    // which keeps the references returned by registerChannel valid.
    std::deque<Channel> channels_;
    std::unordered_map<std::string, size_t> index_;  // name -> position in channels_
};
