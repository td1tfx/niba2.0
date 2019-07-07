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
    void process(nibashared::message_registration &req);
    void process(nibashared::message_login &req);
    void process(nibashared::message_getdata &req);
    void process(nibashared::message_fight &req);
    void process(nibashared::message_createchar &req);
    void process(nibashared::message_learnmagic &req);
    void process(nibashared::message_fusemagic &req);
    void process(nibashared::message_reordermagic &req);
    void process(nibashared::message_echo& msg);            // This is a message
    void process(nibashared::message_send& req);            // Not handling
    const nibashared::sessionstate &get_session() const;

    template<typename Message, typename = nibashared::IsMessage<Message>>
    void process_response(Message &m, const nlohmann::json &merge_j) {
        m.base_merge_response(merge_j);
        session_.current_time = std::chrono::high_resolution_clock::now();
        session_.earliest_time = session_.current_time;
        process(m);
    }

private:
    nibashared::sessionstate session_;
};
} // namespace nibaclient
