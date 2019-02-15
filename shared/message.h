#pragma once

#include "global_defs.h"
#include "sessiondata.h"
#include "structs.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

namespace nibashared {

enum class cmdtype : std::size_t {
    login = 0,
    registeration = 1,
    createchar = 2,
    getdata = 3,
    fight = 4,

    LAST
};

// making staticdata templated might be too difficult to manage

struct message_register {
    const cmdtype type = cmdtype::registeration;

    message_register() = default;
    message_register(std::string &&id, std::string &&password);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    // maybe we can boost hana this as well
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
    std::optional<nibashared::player> player;
    std::vector<nibashared::magic> magics;
    std::vector<nibashared::equipment> equips;
};


struct message_getdata {
    const cmdtype type = cmdtype::getdata;

    message_getdata() = default;
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    std::unordered_map<int, nibashared::character> characters;
    std::unordered_map<int, nibashared::magic> magics;
    std::unordered_map<int, nibashared::equipment> equips;
    bool success = false;
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
    message_createchar(nibashared::player &&player);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    // TODO move all to character after gender is added into character
    nibashared::player player;
    std::vector<nibashared::magic> magics;
    std::vector<nibashared::equipment> equips;
    bool success = false;
};

} // namespace nibashared
