#include "session_map.h"
#include "server_session.h"

#include <iostream>

namespace nibaserver {

bool session_map::write(const std::string &name, std::string &&data) {
    std::lock_guard<std::mutex> guard(mutex_);
    std::cout << "writing\n";
    if (auto iter = map_.find(name); iter != map_.end()) {
        std::cout << name << " is found in session map\n";
        if (auto ptr = iter->second.lock()) {
            ptr->write(std::move(data));
            return true;
        }
    }
    std::cout << "write to " << name << " failed\n";
    return false;
}

void session_map::cleanup() {
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto it = map_.begin(); it != map_.end();) {
        if (it->second.expired()) {
            it = map_.erase(it);
        } else {
            ++it;
        }
    }
}

bool session_map::register_session(const std::string &name, session_wptr wptr) {
    std::lock_guard<std::mutex> guard(mutex_);
    if (auto iter = map_.find(name); iter != map_.end()) {
        if (!iter->second.expired()) {
            return false;
        } else {
            map_.erase(iter);
        }
    }
    map_.emplace(name, wptr);
    std::cout << name << " is online\n";
    return true;
}

} // namespace nibaserver
