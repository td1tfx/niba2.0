#pragma once
#include "db_accessor.h"
#include "global_defs.h"
#include "logger.h"
#include "message.h"
#include <boost/asio/spawn.hpp>

namespace nibaserver {
class server_processor {
public:
    server_processor(boost::asio::yield_context &yield, nibaserver::db_accessor &db);
    ~server_processor() = default;
    void process(nibashared::message_register &req);
    void process(nibashared::message_login &req);
    void process(nibashared::message_fight &req);
    void process(nibashared::message_createchar &req);
    const nibashared::sessionstate &get_session();

private:
    logger logger_;
    nibashared::sessionstate session_;
    boost::asio::yield_context &yield_;
    nibaserver::db_accessor &db_;
};
} // namespace nibaserver
