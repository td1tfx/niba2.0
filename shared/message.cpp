#include "message.h"
#include "gamedata.h"
#include "util.h"
#include <algorithm>
#include <unordered_map>

namespace nibashared {

using nlohmann::json;

message_register::message_register(std::string &&id, std::string &&password) :
    id(std::move(id)), password(std::move(password)) {}

bool message_register::validate(const nibashared::sessionstate &session) {
    // TODO: throw error with error code?
    if (session.state != nibashared::gamestate::prelogin)
        return false;

    if (id.size() < 4 || id.size() > 16 || password.size() < 6 || password.size() > 24) {
        return false;
    }
    return true;
}

json message_register::create_response() { return {}; }

// this is not used
json message_register::create_request() { return {{"id", id}, {"password", password}}; }
void message_register::merge_response(const json &j) { (void)j; }

void message_register::from_request(const json &j) {
    j.at("id").get_to(id);
    j.at("password").get_to(password);
}

message_login::message_login(std::string &&id, std::string &&password) :
    id(std::move(id)), password(std::move(password)) {}

bool message_login::validate(const nibashared::sessionstate &session) {
    if (session.state != nibashared::gamestate::prelogin)
        return false;

    if (id.size() < 4 || id.size() > 16 || password.size() < 6 || password.size() > 24) {
        return false;
    }
    return true;
}

json message_login::create_response() { return {{"player", player}, {"data", data}}; }

// this is not used
json message_login::create_request() { return {{"id", id}, {"password", password}}; }

void message_login::merge_response(const json &j) {
    j.at("player").get_to(player);
    j.at("data").get_to(data);
}

void message_login::from_request(const json &j) {
    j.at("id").get_to(id);
    j.at("password").get_to(password);
}

bool nibashared::message_getdata::validate(const nibashared::sessionstate &session) {
    // TODO change this
    if (session.state != nibashared::gamestate::ingame &&
        session.state != nibashared::gamestate::createchar)
        return false;

    return true;
}

nlohmann::json nibashared::message_getdata::create_response() {
    if (!success) {
        return {};
    }
    return {{"characters", characters}, {"magics", magics}, {"equips", equips}, {"maps", maps}};
}

nlohmann::json nibashared::message_getdata::create_request() { return {}; }

void nibashared::message_getdata::merge_response(const nlohmann::json &j) {
    if (success) {
        j.at("characters").get_to(characters);
        j.at("magics").get_to(magics);
        j.at("equips").get_to(equips);
        j.at("maps").get_to(maps);
    }
}

void nibashared::message_getdata::from_request(const nlohmann::json &j) { (void)j; }

nibashared::message_fight::message_fight(int enemyid) : enemyid(enemyid) {}

bool nibashared::message_fight::validate(const nibashared::sessionstate &session) {
    // TODO change this
    if (session.state != nibashared::gamestate::ingame)
        return false;

    // TODO, remove enemyid
    if (!nibashared::getdata().has<nibashared::character>(enemyid))
        return false;

    return true;
}

nlohmann::json nibashared::message_fight::create_response() { return {{"generated", generated}}; }

nlohmann::json nibashared::message_fight::create_request() { return {{"enemyid", enemyid}}; }

void nibashared::message_fight::merge_response(const nlohmann::json &j) {
    j.at("generated").get_to(generated);
}

void nibashared::message_fight::from_request(const nlohmann::json &j) {
    j.at("enemyid").get_to(enemyid);
}

nibashared::message_createchar::message_createchar(nibashared::player &&player) :
    player(std::move(player)) {}

bool nibashared::message_createchar::validate(const nibashared::sessionstate &session) {
    if (session.state != nibashared::gamestate::createchar)
        return false;
    if (!(player.gender == 'm' || player.gender == 'f'))
        return false;
    if (player.name.empty())
        return false;
    if (player.attrs.strength < 0 || player.attrs.dexterity < 0 || player.attrs.spirit < 0 ||
        player.attrs.physique < 0)
        return false;
    if (player.attrs.strength + player.attrs.dexterity + player.attrs.spirit +
            player.attrs.physique >
        5)
        return false;
    return true;
}

nlohmann::json nibashared::message_createchar::create_response() {
    if (success) {
        return {{"player", player}, {"data", data}};
    }
    return {};
}

nlohmann::json nibashared::message_createchar::create_request() { return {{"player", player}}; }

void nibashared::message_createchar::merge_response(const nlohmann::json &j) {
    if (success) {
        j.at("player").get_to(player);
        j.at("data").get_to(data);
    }
}

void nibashared::message_createchar::from_request(const nlohmann::json &j) {
    j.at("player").get_to(player);
}

nibashared::message_learnmagic::message_learnmagic(int static_id) : static_id(static_id) {}

bool nibashared::message_learnmagic::validate(const nibashared::sessionstate &session) {
    if (session.state != nibashared::gamestate::ingame) {
        return false;
    }

    if (!nibashared::getdata().has<nibashared::magic>(static_id)) {
        return false;
    }

    // can't learn a magic thats already learnt
    if (nibautil::find_if(session.data.magics, [this](auto &magic) {
            return magic.magic_id == static_id;
        }) != session.data.magics.end()) {
        return false;
    }

    // no limit for now
    // max magics size
    // if (session.magics.size() >= 100) {
    //     return false;
    // }

    // TODO: check if player has magic "book" and has enough exp to learn the magic
    // add another table to track magic book?
    // or just add another field in magic?

    return true;
}

nlohmann::json nibashared::message_learnmagic::create_response() {
    if (success) {
        // no extra magic check, we assume that its assigned if success
        // (its actually IFF)
        return {{"magic", magic}};
    }
    return {};
}

nlohmann::json nibashared::message_learnmagic::create_request() {
    return {{"static_id", static_id}};
}

void nibashared::message_learnmagic::merge_response(const nlohmann::json &j) {
    if (success) {
        j.at("magic").get_to(magic);
    }
}

void nibashared::message_learnmagic::from_request(const nlohmann::json &j) {
    j.at("static_id").get_to(static_id);
}

nibashared::message_fusemagic::message_fusemagic(int primary_magic_id, int secondary_magic_id) :
    primary_magic_id(primary_magic_id), secondary_magic_id(secondary_magic_id) {}

bool nibashared::message_fusemagic::validate(const nibashared::sessionstate &session) {
    if (session.state != nibashared::gamestate::ingame)
        return false;

    // player also has to have both magics
    auto primary_iter = nibautil::find_if(
        session.data.magics, [this](auto &magic) { return magic.magic_id == primary_magic_id; });
    auto secondary_iter = nibautil::find_if(
        session.data.magics, [this](auto &magic) { return magic.magic_id == secondary_magic_id; });
    if (primary_iter == session.data.magics.end() || secondary_iter == session.data.magics.end()) {
        return false;
    }
    // must also be the same type
    if (primary_iter->active != secondary_iter->active) {
        return false;
    }
    return true;
}

nlohmann::json nibashared::message_fusemagic::create_response() {
    if (success) {
        return {{"magic", magic}};
    }
    return {};
}

nlohmann::json nibashared::message_fusemagic::create_request() {
    return {{"primary_magic_id", primary_magic_id}, {"secondary_magic_id", secondary_magic_id}};
}

void nibashared::message_fusemagic::merge_response(const nlohmann::json &j) {
    if (success) {
        j.at("magic").get_to(magic);
    }
}

void nibashared::message_fusemagic::from_request(const nlohmann::json &j) {
    j.at("primary_magic_id").get_to(primary_magic_id);
    j.at("secondary_magic_id").get_to(secondary_magic_id);
}

nibashared::message_reordermagic::message_reordermagic(std::vector<int> &&equipped_magic_ids) :
    equipped_magic_ids(std::move(equipped_magic_ids)) {}

bool nibashared::message_reordermagic::validate(const nibashared::sessionstate &session) {
    if (session.state != nibashared::gamestate::ingame)
        return false;
    if (equipped_magic_ids.size() > 6)
        return false;
    // make sure all ids are valid
    // id to index
    std::unordered_map<int, std::size_t> magic_id_set;
    std::size_t idx = 0;
    for (auto &magic : session.data.magics) {
        magic_id_set[magic.magic_id] = idx++;
    }
    std::size_t passives = 0;
    for (auto &id : equipped_magic_ids) {
        auto iter = magic_id_set.find(id);
        if (iter == magic_id_set.end()) {
            return false;
        }
        if (!session.data.magics[iter->second].active) {
            passives++;
        }
    }
    // note 5 actives and 1 passive
    if (passives > 1)
        return false;
    if ((equipped_magic_ids.size() - passives) > 5)
        return false;
    return true;
}

nlohmann::json nibashared::message_reordermagic::create_response() { return {}; }

nlohmann::json nibashared::message_reordermagic::create_request() {
    return {{"equipped_magic_ids", equipped_magic_ids}};
}

void nibashared::message_reordermagic::merge_response(const nlohmann::json &j) { (void)j; }

void nibashared::message_reordermagic::from_request(const nlohmann::json &j) {
    j.at("equipped_magic_ids").get_to(equipped_magic_ids);
}

} // namespace nibashared