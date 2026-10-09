#include "blackboard.h"

bool Blackboard::write(const std::string& name, const Value& value) {
    auto it = index_.find(name);
    if (it == index_.end()) return false;
    Channel& ch = channels_[it->second];
    if (ch.value.index() != value.index()) return false;
    ch.value = value;
    return true;
}
