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
            nibashared::cmdtype cmd =
                static_cast<nibashared::cmdtype>(j.at("type").get<std::size_t>());
            switch (cmd) {
            case nibashared::cmdtype::registeration: {
                nibashared::register_request req;
                req.from_json(j);
                return do_request_(req);
            }
            case nibashared::cmdtype::login: {
                nibashared::login_request req;
                req.from_json(j);
                return do_request_(req);
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
    std::string do_request_(request& req) {
        if (!req.validate(processor_.get_session())) {
            throw std::exception("validation failure");
        }
        processor_.process(req);
        return req.create_response().dump();
    }
    processor& processor_;
};

}