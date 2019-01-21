#include "server_processor.h"
#include "db_accessor.h"
#include "fight.h"
#include "server_gamedata.h"
#include "rng.h"

#include <iostream>

using namespace nibaserver;
namespace sev = boost::log::trivial;

server_processor::server_processor(boost::asio::yield_context &yield, nibaserver::db_accessor &db) :
    yield_(yield), db_(db) {
    session_.state = nibashared::gamestate::prelogin;
}

std::string server_processor::dispatch(const std::string &request) {
    try {
        auto j = nlohmann::json::parse(request);
        auto cmd_id = j.at("type").get<std::size_t>();
        switch (static_cast<nibashared::cmdtype>(cmd_id)) {
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
            BOOST_LOG_SEV(logger_, sev::info) << "unknown request type: " << cmd_id;
        }
    }
    // return whatever error message, I don't care
    catch (std::exception &e) {
        BOOST_LOG_SEV(logger_, sev::info) << "dispatch failure: " << e.what();
    }
    nlohmann::json error_msg{{"error", "request error"}};
    return error_msg.dump();
}

void nibaserver::server_processor::process(nibashared::message_register &req) {
    if (db_.create_user(yield_, req.id, req.password)) {
        req.success = true;
        BOOST_LOG_SEV(logger_, sev::info) << "User " << req.id << " has registered.";
    } else {
        req.success = false;
        BOOST_LOG_SEV(logger_, sev::warning) << "User " << req.id << " failed to register.";
    }
}

void nibaserver::server_processor::process(nibashared::message_login &req) {
    if (db_.login(yield_, req.id, req.password)) {
        req.success = true;
        session_.state = nibashared::gamestate::createchar;
        session_.userid = req.id;
        req.characters = {"niba1", "niba2"};
    } else {
        req.success = false;
    }
    // TODO change this later
    session_.charid = 0;
    BOOST_LOG_SEV(logger_, sev::info) << "User " << req.id << " logging in is " << req.success;
}

void nibaserver::server_processor::process(nibashared::message_fight &req) {
    auto [self_fightable, enemy_fightable] = nibashared::prep_fight<nibashared::server_staticdata>(5, 7);
    // session_.charid, req.enemyid
    nibashared::fight fight(std::move(self_fightable), std::move(enemy_fightable));
    nibashared::rng_server rng;
    fight.go(rng);
    req.generated = std::move(rng.generated);
}

void nibaserver::server_processor::process(nibashared::message_createchar &req) {
    if (!session_.userid) {
        req.success = false;
        return;
    }
    // players have an id of -1? or auto increment?
    nibashared::character c{req.name, -1, req.attrs, {}, {}, {}};
    if (db_.create_char(*(session_.userid), std::move(c))) {
        req.success = true;
        session_.state = nibashared::gamestate::ingame;
        BOOST_LOG_SEV(logger_, sev::info)
            << "User " << *(session_.userid) << " created an character " << req.name;
    } else {
        req.success = false;
    }
}

const nibashared::sessionstate &nibaserver::server_processor::get_session() { return session_; }
