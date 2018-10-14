#pragma once

#include <unordered_map>
#include <string>

// just a in-memory map for now

namespace nibaserver {

class db_accessor
{
public:
    struct user {
        bool logged_in;
        // int salt; // do it later
        unsigned char salt[32];
        unsigned char hashed_password[32];
    };

    static bool login(const std::string& id, const std::string& password);
    static bool logout(const std::string& id);
    static bool create_user(const std::string& id, const std::string& password);
private:
    // TODO: add mutex, but whatever, probably have a db running before that
    static std::unordered_map<std::string, user> db_;
};

}
