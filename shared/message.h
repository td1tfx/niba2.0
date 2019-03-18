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
    learnmagic = 5,
    fusemagic = 6,
    reordermagic = 7,

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
    std::optional<nibashared::playerdata> data;
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

    nibashared::player player;
    nibashared::playerdata data;    // this is part of response
    bool success = false;
};

struct message_learnmagic {
    // A magic_book is a book id stored in player_books
    // a player learns a magic from a magic_book
    // but a magic_book is just a magic in staticdata(TODO: add exp requirement)
    const cmdtype type = cmdtype::learnmagic;

    message_learnmagic() = default;
    message_learnmagic(int static_id);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    int static_id;
    nibashared::magic magic;
    bool success = false;
};

struct message_fusemagic {
    const cmdtype type = cmdtype::fusemagic;

    message_fusemagic() = default;
    message_fusemagic(int primary_magic_id, int secondary_magic_id);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    int primary_magic_id;
    int secondary_magic_id;
    nibashared::magic magic;
    bool success = false;
};

struct message_reordermagic {
    // use the same message for equipping & unequipping magics
    const cmdtype type = cmdtype::reordermagic;

    message_reordermagic() = default;
    message_reordermagic(std::vector<int>&& equipped_magic_ids);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    std::vector<int> equipped_magic_ids;
    bool success = false;
};

} // namespace nibashared
