#pragma once

#include "global_defs.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace nibashared {

class register_request {
public:
    const cmdtype type = cmdtype::registeration;

    register_request() = default;
    register_request(const std::string &id, const std::string &password);
    bool validate(nibashared::sessionstate session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_json(const nlohmann::json &j);

    std::string id;
    std::string password;
    bool success = false;
};

class login_request {
public:
    const cmdtype type = cmdtype::login;

    login_request() = default;
    login_request(const std::string &id, const std::string &password);
    bool validate(nibashared::sessionstate session);
    nlohmann::json create_response();
    nlohmann::json create_request();
    void merge_response(const nlohmann::json &j);
    void from_json(const nlohmann::json &j);

    std::string id;
    std::string password;
    bool success = false;
    std::vector<std::string> characters;
};

} // namespace nibashared