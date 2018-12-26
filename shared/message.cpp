#include "message.h"

using nlohmann::json;
using namespace nibashared;

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

json message_register::create_response() { return {{"success", success}}; }

// this is not used
json message_register::create_request() {
    return {{"type", type}, {"id", id}, {"password", password}};
}
void message_register::merge_response(const json &j) { j.at("success").get_to(success); }

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

json message_login::create_response() { return {{"success", success}, {"characters", characters}}; }

// this is not used
json message_login::create_request() {
    return {{"type", type}, {"id", id}, {"password", password}};
}
void message_login::merge_response(const json &j) {
    j.at("success").get_to(success);
    j.at("characters").get_to(characters);
}

void message_login::from_request(const json &j) {
    j.at("id").get_to(id);
    j.at("password").get_to(password);
}

nibashared::message_fight::message_fight(int enemyid) : enemyid(enemyid) {}

bool nibashared::message_fight::validate(const nibashared::sessionstate &session) {
    // TODO change this
    if (session.state != nibashared::gamestate::ingame)
        return false;

    // currently only support id=1
    if (enemyid != 1)
        return false;

    return true;
}

nlohmann::json nibashared::message_fight::create_response() { return {{"generated", generated}}; }

nlohmann::json nibashared::message_fight::create_request() {
    return {{"type", type}, {"enemyid", enemyid}};
}

void nibashared::message_fight::merge_response(const nlohmann::json &j) {
    j.at("generated").get_to(generated);
}

void nibashared::message_fight::from_request(const nlohmann::json &j) {
    j.at("enemyid").get_to(enemyid);
}

nibashared::message_createchar::message_createchar(std::string &&name, int gender,
                                                   attributes &&attrs) :
    name(std::move(name)),
    gender{gender}, attrs{std::move(attrs)} {}

bool nibashared::message_createchar::validate(const nibashared::sessionstate &session) {
    if (session.state != nibashared::gamestate::createchar)
        return false;
    if (!(gender == 0 || gender == 1))
        return false;
    if (name.empty())
        return false;
    if (attrs.strength < 0 || attrs.dexterity < 0 || attrs.spirit < 0 || attrs.physique < 0)
        return false;
    if (attrs.strength + attrs.dexterity + attrs.spirit + attrs.physique > 5)
        return false;
    return true;
}

nlohmann::json nibashared::message_createchar::create_response() {
    // TODO maybe return the character data as well
    return {{"success", success}};
}

nlohmann::json nibashared::message_createchar::create_request() {
    return {{"type", type}, {"name", name}, {"gender", gender}, {"attrs", attrs}};
}

void nibashared::message_createchar::merge_response(const nlohmann::json &j) {
    j.at("success").get_to(success);
}

void nibashared::message_createchar::from_request(const nlohmann::json &j) {
    j.at("name").get_to(name);
    j.at("gender").get_to(gender);
    j.at("attrs").get_to(attrs);
}
