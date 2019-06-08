#pragma once

#include "logger.h"

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace nibaserver {

class server_session;

// A map of user_id -> session ptr
// Needs to be thread safe

// forward declare as server_session also need a session_map

class session_map {
public:
    using session_wptr = std::weak_ptr<server_session>;

    session_map() = default;
    session_map(const session_map &) = delete;
    session_map &operator=(const session_map &) = delete;

    // Write to session of given name, returns whether person is online
    // (eg. session exists and not yet destroyed)
    bool write(const std::string &name, std::string &&data);

    // Clean up, remove empty weak_ptrs, need a timer to run this periodically
    // not a huge concern for now
    void cleanup();

    // Register, session
    // Once user logs in properly, add to map_
    // if a valid entry exists, then deny the login
    bool register_session(const std::string &name, session_wptr wptr);

    // Attempt to remove a session, usually the session itself on destruction
    void remove(const std::string& name);

private:
    std::shared_mutex mutex_;
    std::unordered_map<std::string, session_wptr> map_;
    logger logger_;
};

} // namespace nibaserver