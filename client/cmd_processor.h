#pragma once
#include "global_defs.h"
#include "client_processor.h"
#include "message.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace nibaclient {
    class cmd_processor
    {
    public:
        template<typename handler>
        static void handle_cmd(std::string& input, nibaclient::client_processor& processor, handler handler) {
            auto results = splitter(input);
            // password input handled by cmd_processor
            if (results.size() == 3) {
                if (results[0] == "register") {
                    nibashared::register_request request(results[1], results[2]);
                    return validate_and_go(processor, request, handler);
                }
                else if (results[0] == "login") {
                    nibashared::login_request request(results[1], results[2]);
                    return validate_and_go(processor, request, handler);
                }
            }
            std::cout << "incorrect command" << std::endl;
        }

    private:
        template<typename message, typename handler>
        static void validate_and_go(nibaclient::client_processor& processor,
            message& message, handler handler) {
            if (!message.validate(processor.get_session())) {
                // TODO maybe message specific error msg
                std::cout << "command validation failed" << std::endl;
            }
            else {
                handler(message);
            }
        }

        static std::vector<std::string> splitter(const std::string& s);
    };
}


