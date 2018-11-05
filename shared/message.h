#pragma once

#include "global_defs.h"
#include "structs.h"
#include <nlohmann/json.hpp>
#include <vector>

namespace nibashared {

enum class cmdtype : std::size_t {
    login = 0,
    registeration = 1,
    createchar = 2,
    // start = 3,
    fight = 4,

    LAST
};

struct message_register {
    const cmdtype type = cmdtype::registeration;

    message_register() = default;
    message_register(std::string &&id, std::string &&password);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    std::string id;
    std::string password;
    bool success = false;
};
// TODO this will help serialization with more complex stuff
// void from_json(const nlohmann::json &j, register_request &req);
// void to_json(json &j, const register_request &req);

struct message_login {
    const cmdtype type = cmdtype::login;

    message_login() = default;
    message_login(std::string &&id, std::string &&password);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    std::string id;
    std::string password;
    bool success = false;
    std::vector<std::string> characters;
};

struct message_fight {
    const cmdtype type = cmdtype::fight;

    message_fight() = default;
    // TODO this should be changed, we shouldn't use enemyid to identify the enemy
    // or more validation is needed
    message_fight(int enemyid);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    int enemyid;
    std::vector<int> generated;
};

struct message_createchar {
    const cmdtype type = cmdtype::createchar;

    message_createchar() = default;
    message_createchar(std::string &&name, int gender, attributes &&attrs);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    std::string name;
    int gender;
    attributes attrs;
    bool success = false;
};

} // namespace nibashared
