#pragma once

#include "global_defs.h"
#include "sessiondata.h"
#include "structs.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace nibashared {

namespace message {

enum class type : int {
    none = -1,
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

enum class tag : int { request = 0, response = 1 };

} // namespace message

template<typename Impl>
struct base_message {
    Impl &self() { return static_cast<Impl &>(*this); }
    bool base_validate(const nibashared::sessionstate &session) { return self().validate(session); }
    nlohmann::json base_create_response() {
        auto j = self().create_response();
        j["success"] = success;
        j["type"] = self().type;
        j["tag"] = message::tag::response;
        return j;
    }
    nlohmann::json base_create_request() {
        auto j = self().create_request();
        j["tag"] = message::tag::request;
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

struct message_registration : public base_message<message_registration> {
    // TODO: make this static?
    const message::type type = message::type::registeration;

    message_registration() = default;
    message_registration(std::string &&id, std::string &&password);
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
    const message::type type = message::type::login;

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
    const message::type type = message::type::getdata;

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
    const message::type type = message::type::fight;

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
    const message::type type = message::type::createchar;

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
    const message::type type = message::type::learnmagic;

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
    const message::type type = message::type::fusemagic;

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
    const message::type type = message::type::reordermagic;

    message_reordermagic() = default;
    explicit message_reordermagic(std::vector<int> &&equipped_magic_ids);
    bool validate(const nibashared::sessionstate &session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_request(const nlohmann::json &j);

    std::vector<int> equipped_magic_ids;
};

namespace message {

using variant =
    std::variant<message_registration, message_login, message_getdata, message_fight,
                 message_createchar, message_learnmagic, message_fusemagic, message_reordermagic>;

template<typename Message, typename = nibashared::IsMessage<Message>>
Message parse(const nlohmann::json &j) {
    Message m;
    m.base_from_request(j);
    return m;
}

template<typename Handler>
auto dispatcher(const nlohmann::json &j, Handler &&handler) {
    auto cmd_id = j.at("type").get<int>();
    switch (message::type{cmd_id}) {
    case message::type::registeration: {
        return handler(parse<message_registration>(j));
    }
    case message::type::login: {
        return handler(parse<message_login>(j));
    }
    case message::type::getdata: {
        return handler(parse<message_getdata>(j));
    }
    case message::type::fight: {
        return handler(parse<message_fight>(j));
    }
    case message::type::createchar: {
        return handler(parse<message_createchar>(j));
    }
    case message::type::learnmagic: {
        return handler(parse<message_learnmagic>(j));
    }
    case message::type::fusemagic: {
        return handler(parse<message_fusemagic>(j));
    }
    case message::type::reordermagic: {
        return handler(parse<message_reordermagic>(j));
    }
    default:
        throw std::runtime_error("unknown request " + std::to_string(cmd_id));
    }
}

} // namespace message

} // namespace nibashared
