#pragma once
#include "global_defs.h"
#include "message.h"
#include "sessiondata.h"
#include <iostream>
#include <nlohmann/json.hpp>

namespace nibaclient {
class client_processor {
public:
    client_processor();
    ~client_processor() = default;
    void process(nibashared::message_register &req);
    void process(nibashared::message_login &req);
    void process(nibashared::message_getdata &req);
    void process(nibashared::message_fight &req);
    void process(nibashared::message_createchar &req);
    void process(nibashared::message_learnmagic &req);
    void process(nibashared::message_fusemagic &req);
    void process(nibashared::message_reordermagic &req);
    const nibashared::sessionstate &get_session();

    template<typename message>
    void dispatch(message &m, const std::string &merger) {
        std::cout << merger << std::endl;
        try {
            auto merge_j = nlohmann::json::parse(merger);
            if (merge_j.find("error") != merge_j.end()) {
                std::string err = merge_j["error"].get<std::string>();
                throw std::runtime_error(err.c_str());
            }
            m.merge_response(merge_j);
            process(m);
        } catch (std::exception &e) {
            std::cout << "Client server communication failed: " << e.what() << std::endl;
        }
    }

private:
    nibashared::sessionstate session_;
};
} // namespace nibaclient
