#pragma once
#include "global_defs.h"
#include "logger.h"
#include "message.h"
#include <boost/asio/spawn.hpp>

namespace nibaserver {
class server_processor {
public:
    server_processor(boost::asio::io_context &ioc, boost::asio::yield_context &yield);
    ~server_processor() = default;
    void process(nibashared::message_register &req);
    void process(nibashared::message_login &req);
    void process(nibashared::message_fight &req);
    void process(nibashared::message_createchar &req);
    const nibashared::sessionstate &get_session();

private:
    logger logger_;
    nibashared::sessionstate session_;
    boost::asio::io_context &ioc_;
    boost::asio::yield_context &yield_;
};
} // namespace nibaserver
