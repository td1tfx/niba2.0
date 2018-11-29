#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include "structs.h"
#include "logger.h"
#include <boost/asio/spawn.hpp>
#include <ozo/request.h>
#include <ozo/connection_info.h>
#include <ozo/shortcuts.h>
#include <ozo/execute.h>
#include <ozo/connection.h>
// just a in-memory map for now

namespace nibaserver {

class db_accessor {
public:
    struct user {
        bool logged_in;
        // int salt; // do it later
        std::vector<char> salt;
        std::vector<char> hashed_password;
        // not the best database representation perhaps
        std::optional<std::string> char_name;
        user() {
            salt.resize(32, 0);
            hashed_password.resize(32, 0);
        }
    };

    static bool login(boost::asio::io_context &ioc, boost::asio::yield_context &yield,
        const std::string &id, const std::string &password);

    static bool logout(boost::asio::io_context &ioc, boost::asio::yield_context &yield, const std::string &id);
    
    static bool create_user(boost::asio::io_context &ioc, boost::asio::yield_context &yield,
        const std::string &id, const std::string &password);

    // NOTE no save char functions yet
    static std::optional<nibashared::character> get_char(const std::string &id);
    static bool create_char(const std::string &id, nibashared::character&& character);

private:
    // TODO: add mutex, but whatever, probably have a db running before that
    static logger logger_;
    static std::unordered_map<std::string, nibashared::character> char_tbl_;
};

} // namespace nibaserver
