#include "request_dispatcher.h"

using namespace nibaserver;

request_dispatcher::request_dispatcher(server_processor &proc) : processor_(proc) {}

std::string request_dispatcher::dispatch(const std::string &request) {
    try {
        // std::cout << request << std::endl;
        auto j = nlohmann::json::parse(request);
        auto cmd = static_cast<nibashared::cmdtype>(j.at("type").get<std::size_t>());
        switch (cmd) {
        case nibashared::cmdtype::registeration: {
            return do_request<nibashared::message_register>(j);
        }
        case nibashared::cmdtype::login: {
            return do_request<nibashared::message_login>(j);
        }
        case nibashared::cmdtype::fight: {
            return do_request<nibashared::message_fight>(j);
        }
        case nibashared::cmdtype::createchar: {
            return do_request<nibashared::message_createchar>(j);
        }
        default:
            throw std::runtime_error("invalid request");
        }
    }
    // return whatever error message, I don't care
    catch (std::exception &e) {
        // TODO: use glog
        std::cout << e.what() << std::endl;
    }
    nlohmann::json error_msg{{"error", "request error"}};
    return error_msg.dump();
}