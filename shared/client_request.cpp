#include "client_request.h"
#include "utils.h"

using nlohmann::json;
using namespace nibashared;

register_request::register_request(const std::string & id, const std::string & password) :
    id(id), password(password)
{
}

bool register_request::validate(nibashared::sessionstate session)
{
    // TODO: probably throw errors?
    if (!nibashared::is_cmd_valid(session.state, cmdtype::registeration)) {
        return false;
    }
    if (id.size() < 4 || id.size() > 16 || password.size() < 6 || password.size() > 24) {
        return false;
    }
    return true;
}

json register_request::create_response()
{
    json j = {
        {"success", success}
    };
    return j;
}

// this is not used
json register_request::create_request()
{
    json j = {
        {"type", type},
        {"id", id},
        {"password", password}
    };
    return j;
}
void register_request::merge_response(const json & j)
{
    j.at("success").get_to(success);
}

void register_request::from_json(const json & j)
{
    j.at("id").get_to(id);
    j.at("password").get_to(password);
}


login_request::login_request(const std::string & id, const std::string & password) :
    id(id), password(password)
{
}

bool login_request::validate(nibashared::sessionstate session)
{
    if (!nibashared::is_cmd_valid(session.state, cmdtype::registeration)) {
        return false;
    }
    if (id.size() < 4 || id.size() > 16 || password.size() < 6 || password.size() > 24) {
        return false;
    }
    return true;
}

json login_request::create_response()
{
    json j = {
        {"success", success},
        {"characters", characters}
    };
    return j;
}

// this is not used
json login_request::create_request()
{
    json j = {
        {"type", type},
        {"id", id},
        {"password", password}
    };
    return j;
}
void login_request::merge_response(const json & j)
{
    j.at("success").get_to(success);
    j.at("characters").get_to(characters);
}

void login_request::from_json(const json & j)
{
    j.at("id").get_to(id);
    j.at("password").get_to(password);
}
