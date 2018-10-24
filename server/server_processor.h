#pragma once
#include "global_defs.h"
#include "logger.h"
#include "message.h"

namespace nibaserver {
class server_processor {
public:
    nibaserver::logger logger;
    server_processor();
    ~server_processor() = default;
    void process(nibashared::message_register &req);
    void process(nibashared::message_login &req);
    void process(nibashared::message_fight &req);
    const nibashared::sessionstate &get_session();

private:
    nibashared::sessionstate session;
};
} // namespace nibaserver
