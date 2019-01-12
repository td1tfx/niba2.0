#pragma once

#include "message.h"
#include "server_processor.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

namespace nibaserver {

class request_dispatcher {
public:
    request_dispatcher(server_processor &proc);
    // dispatch is responsible to handle all the type conversion, make the whatever calls
    // and then finally return the serialized message
    std::string dispatch(const std::string &request);

private:
    server_processor &processor_;

    template<typename request>
    std::string do_request(const nlohmann::json &json_request) {
        request req;
        req.from_request(json_request);
        if (!req.validate(processor_.get_session())) {
            throw std::runtime_error("validation failure");
        }
        processor_.process(req);
        return req.create_response().dump();
    }
};

} // namespace nibashared
