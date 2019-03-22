#pragma once
#include "global_defs.h"
#include "message.h"
#include "sessiondata.h"
#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>

namespace nibaclient {
class client_processor {
public:
    client_processor() = default;
    ~client_processor() = default;
    void process(nibashared::message_register &req);
    void process(nibashared::message_login &req);
    void process(nibashared::message_getdata &req);
    void process(nibashared::message_fight &req);
    void process(nibashared::message_createchar &req);
    void process(nibashared::message_learnmagic &req);
    void process(nibashared::message_fusemagic &req);
    void process(nibashared::message_reordermagic &req);
    const nibashared::sessionstate &get_session() const;

    template<typename Message, typename = std::enable_if_t<std::is_base_of<
                                   nibashared::base_message<Message>, Message>::value>>
    void dispatch(Message &m, const std::string &merger) {
        std::cout << merger << std::endl;
        try {
            auto merge_j = nlohmann::json::parse(merger);
            if (merge_j.find("error") != merge_j.end()) {
                std::string err = merge_j["error"].get<std::string>();
                throw std::runtime_error(err.c_str());
            }
            m.base_merge_response(merge_j);
            session_.current_time = std::chrono::high_resolution_clock::now();
            session_.earliest_time = session_.current_time;
            process(m);
        } catch (std::exception &e) {
            std::cout << "Client server communication failed: " << e.what() << std::endl;
        }
    }

private:
    nibashared::sessionstate session_;
};
} // namespace nibaclient
