#pragma once

#include "global_defs.h"
#include "sessiondata.h"
#include "structs.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

namespace nibashared {

enum class type_message : std::size_t {
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

enum class tag_message : std::size_t { request = 0, response = 1 };

template<typename Impl>
struct base_message {
    Impl &self() { return static_cast<Impl &>(*this); }
    bool base_validate(const nibashared::sessionstate &session) { return self().validate(session); }
    nlohmann::json base_create_response() {
        auto j = self().create_response();
        j["success"] = success;
        j["tag"] = tag_message::response;
        return j;
    }
    nlohmann::json base_create_request() {
        auto j = self().create_request();
        j["tag"] = tag_message::request;
        j["type"] = self().type;
        return j;
    }
    void base_merge_response(const nlohmann::json &j) {
        j.at("success").get_to(success);
        self().merge_response(j);
    }
    void base_from_request(const nlohmann::json &j) { self().from_request(j); }

    bool success = false;
};

template<typename T>
using IsMessage = typename std::enable_if<std::is_base_of<base_message<T>, T>::value>::type;

struct message_register : public base_message<message_register> {
    // TODO: make this static?
    const type_message type = type_message::registeration;

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
};

struct message_login : public base_message<message_login> {
    const type_message type = type_message::login;

    message_login() = default;
    message_login(std::string &&id, std::string &&password);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    std::string id;
    std::string password;

    std::optional<nibashared::player> player;
    std::optional<nibashared::playerdata> data;
};

struct message_getdata : public base_message<message_getdata> {
    const type_message type = type_message::getdata;

    message_getdata() = default;
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    std::unordered_map<int, nibashared::character> characters;
    std::unordered_map<int, nibashared::magic> magics;
    std::unordered_map<int, nibashared::equipment> equips;
    std::unordered_map<int, nibashared::map> maps;
};

struct message_fight : public base_message<message_fight> {
    const type_message type = type_message::fight;

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

struct message_createchar : public base_message<message_createchar> {
    const type_message type = type_message::createchar;

    message_createchar() = default;
    message_createchar(nibashared::player &&player);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    nibashared::player player;
    nibashared::playerdata data; // this is part of response
};

struct message_learnmagic : public base_message<message_learnmagic> {
    // A magic_book is a book id stored in player_books
    // a player learns a magic from a magic_book
    // but a magic_book is just a magic in staticdata(TODO: add exp requirement)
    const type_message type = type_message::learnmagic;

    message_learnmagic() = default;
    explicit message_learnmagic(int static_id);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    int static_id;
    nibashared::magic magic;
};

struct message_fusemagic : public base_message<message_fusemagic> {
    const type_message type = type_message::fusemagic;

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
};

struct message_reordermagic : public base_message<message_reordermagic> {
    // use the same message for equipping & unequipping magics
    const type_message type = type_message::reordermagic;

    message_reordermagic() = default;
    explicit message_reordermagic(std::vector<int> &&equipped_magic_ids);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    std::vector<int> equipped_magic_ids;
};

using variant_message =
    std::variant<message_register, message_login, message_getdata, message_fight,
                 message_createchar, message_learnmagic, message_fusemagic, message_reordermagic>;

} // namespace nibashared
