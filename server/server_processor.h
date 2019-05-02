#pragma once
#include "db_accessor.h"
#include "global_defs.h"
#include "logger.h"
#include "message.h"
#include "sessiondata.h"

#include <boost/asio/spawn.hpp>
#include <chrono>
#include <nlohmann/json.hpp>

namespace nibaserver {
class server_processor {
public:
    server_processor(boost::asio::yield_context &yield, nibaserver::db_accessor &db);
    ~server_processor() = default;
    // dispatch is responsible to handle all the type conversion, make the whatever calls
    // and then finally return the serialized message
    std::string dispatch(const std::string &request);
    // process needed for all messages
    void process(nibashared::message_registration &req);
    void process(nibashared::message_login &req);
    void process(nibashared::message_getdata &req);
    void process(nibashared::message_fight &req);
    void process(nibashared::message_createchar &req);
    void process(nibashared::message_learnmagic &req);
    void process(nibashared::message_fusemagic &req);
    void process(nibashared::message_reordermagic &req);
    const nibashared::sessionstate &get_session();

private:
    logger logger_;
    nibashared::sessionstate session_;
    boost::asio::yield_context &yield_;
    nibaserver::db_accessor &db_;
};
} // namespace nibaserver
