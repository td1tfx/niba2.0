#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include "structs.h"
// just a in-memory map for now

namespace nibaserver {

class db_accessor {
public:
    struct user {
        bool logged_in;
        // int salt; // do it later
        unsigned char salt[32];
        unsigned char hashed_password[32];
        // not the best database representation perhaps
        std::optional<std::string> char_name;
    };

    static bool login(const std::string &id, const std::string &password);
    static bool logout(const std::string &id);
    static bool create_user(const std::string &id, const std::string &password);

    // NOTE no save char functions yet
    static std::optional<nibashared::character> get_char(const std::string &id);
    static bool create_char(const std::string &id, nibashared::character&& character);

private:
    // TODO: add mutex, but whatever, probably have a db running before that
    static std::unordered_map<std::string, user> db_;
    static std::unordered_map<std::string, nibashared::character> char_tbl_;
};

} // namespace nibaserver
