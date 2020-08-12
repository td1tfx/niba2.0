#include "session_map.h"
#include "server_session.h"

#include <iostream>

namespace nibaserver {

bool session_map::write(const std::string &name, std::string &&data) {
    std::shared_lock lock{mutex_};
    if (auto iter = map_.find(name); iter != map_.end()) {
        if (auto ptr = iter->second.lock()) {
            ptr->write(name, std::move(data));
            return true;
        }
    }
    return false;
}

void session_map::cleanup() {
    std::unique_lock lock{mutex_};
    for (auto it = map_.begin(); it != map_.end();) {
        if (it->second.expired()) {
            it = map_.erase(it);
        } else {
            ++it;
        }
    }
}

bool session_map::register_session(const std::string &name, session_wptr wptr) {
    std::unique_lock lock{mutex_};
    if (auto iter = map_.find(name); iter != map_.end()) {
        if (!iter->second.expired()) {
            return false;
        } else {
            map_.erase(iter);
        }
    }
    map_.emplace(name, wptr);
    return true;
}

void session_map::remove(const std::string& name) {
    std::unique_lock lock{mutex_};
    if (auto iter = map_.find(name); iter != map_.end()) {
        map_.erase(iter);
    }
}

} // namespace nibaserver
