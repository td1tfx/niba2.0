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
    void operator()(nibashared::message_registration &req);
    void operator()(nibashared::message_login &req);
    void operator()(nibashared::message_getdata &req);
    void operator()(nibashared::message_fight &req);
    void operator()(nibashared::message_createchar &req);
    void operator()(nibashared::message_learnmagic &req);
    void operator()(nibashared::message_fusemagic &req);
    void operator()(nibashared::message_reordermagic &req);
    const nibashared::sessionstate &get_session() const;

    template<typename Message, typename = nibashared::IsMessage<Message>>
    void process_response(Message &m, const nlohmann::json &merge_j) {
        try {
            m.base_merge_response(merge_j);
            session_.current_time = std::chrono::high_resolution_clock::now();
            session_.earliest_time = session_.current_time;
            operator()(m);
        } catch (std::exception &e) {
            std::cout << "Failed to process response: " << e.what() << std::endl;
        }
    }

private:
    nibashared::sessionstate session_;
};
} // namespace nibaclient
