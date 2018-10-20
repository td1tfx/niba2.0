#pragma once

#include "message.h"

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>

namespace nibashared {

// read json string, figure out which request this, check if the request can be accepted
// at the current game state
template<typename processor>
class request_dispatcher
{
public:
    request_dispatcher(processor & proc) : processor_(proc)
    {
    }
    // dispatch is responsible to handle all the type conversion, make the whatever calls
    // and then finally return the serialized message
    std::string dispatch(const std::string & request)
    {
        // if exceptions happen
        try {
            // std::cout << request << std::endl;
            auto j = nlohmann::json::parse(request);
            auto cmd = static_cast<nibashared::cmdtype>(j.at("type").get<std::size_t>());
            switch (cmd) {
            case nibashared::cmdtype::registeration: {
                return do_request<nibashared::register_request>(j);
            }
            case nibashared::cmdtype::login: {
                return do_request<nibashared::login_request>(j);
            }
            }
            throw std::exception("invalid request");
        }
        // return whatever error message, I don't care
        catch (std::exception& e) {
            // TODO: use glog
            std::cout << e.what() << std::endl;
        }
        nlohmann::json error_msg{
            {"error", "request error"}
        };
        return error_msg.dump();
    }
private:
    template<typename request>
    std::string do_request(const nlohmann::json & json_request) {
        request req;
        req.from_json(json_request);
        if (!req.validate(processor_.get_session())) {
            throw std::exception("validation failure");
        }
        processor_.process(req);
        return req.create_response().dump();
    }
    processor& processor_;
};

}