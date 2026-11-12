#include "blackboard.h"

#include <Arduino.h>
#include <cstdio>
#include <type_traits>

namespace {

// Write one value as text, snprintf-style: returns the length it needed
int formatValue(char* out, size_t cap, const Value& value) {
    return std::visit(
        [&](const auto& v) -> int {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return snprintf(out, cap, "%s", v.c_str());
            } else if constexpr (std::is_same_v<T, uint32_t>) {
                return snprintf(out, cap, "%lu", static_cast<unsigned long>(v));
            } else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int>) {
                return snprintf(out, cap, "%d", static_cast<int>(v));
            } else {
                return snprintf(out, cap, "%.3f", static_cast<double>(v));
            }
        },
        value);
}

}  // namespace

size_t Blackboard::pack(char* out, size_t cap) const {
    if (cap == 0) return 0;
    out[0] = '\0';
    size_t len = 0;
    for (const Channel& ch : channels_) {
        size_t start = len;
        int n = snprintf(out + len, cap - len, "%s%s=", len ? "," : "", ch.name.c_str());
        if (n < 0 || static_cast<size_t>(n) >= cap - len) {
            out[start] = '\0';
            return start;
        }
        len += n;
        n = formatValue(out + len, cap - len, ch.value);
        if (n < 0 || static_cast<size_t>(n) >= cap - len) {
            out[start] = '\0';
            return start;
        }
        len += n;
    }
    return len;
}

void Blackboard::halt(const std::string& msg) {
    while (true) {
        Serial.println(msg.c_str());
        delay(1000);
    }
}

bool Blackboard::write(const std::string& name, const Value& value) {
    auto it = index_.find(name);
    if (it == index_.end()) return false;
    Channel& ch = channels_[it->second];
    if (ch.value.index() != value.index()) return false;
    ch.value = value;
    return true;
}
