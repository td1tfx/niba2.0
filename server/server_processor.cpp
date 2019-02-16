#include "server_processor.h"
#include "db_accessor.h"
#include "fight.h"
#include "rng.h"
#include "gamedata.h"

#include <iostream>

using namespace nibaserver;
namespace sev = boost::log::trivial;

server_processor::server_processor(boost::asio::yield_context &yield, nibaserver::db_accessor &db) :
    session_(), yield_(yield), db_(db) {}

std::string server_processor::dispatch(const std::string &request) {
    // logging the request is bad, when password is involved
    // BOOST_LOG_SEV(logger_, sev::debug) << request;
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
        case nibashared::cmdtype::getdata: {
            return do_request<nibashared::message_getdata>(j);
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
    if (db_.create_user(req.id, req.password, yield_)) {
        req.success = true;
        BOOST_LOG_SEV(logger_, sev::info) << "User " << req.id << " has registered.";
    } else {
        req.success = false;
        BOOST_LOG_SEV(logger_, sev::warning) << "User " << req.id << " failed to register.";
    }
}

void nibaserver::server_processor::process(nibashared::message_login &req) {
    if (db_.login(req.id, req.password, yield_)) {
        req.success = true;
        session_.userid = req.id;
        req.player = db_.get_char(req.id, yield_);
        if (req.player) {
            session_.state = nibashared::gamestate::ingame;
            session_.player = *(req.player);
            // player data exists, so lets fetch equipments&magics data of the player
            std::tie(session_.magics, session_.equips) = db_.get_aux((*(req.player)).name, yield_);
            req.magics = session_.magics;
            req.equips = session_.equips;
        } else {
            session_.state = nibashared::gamestate::createchar;
        }
    } else {
        req.success = false;
    }
    BOOST_LOG_SEV(logger_, sev::info) << "User " << req.id << " logging in is " << req.success;
}

void nibaserver::server_processor::process(nibashared::message_getdata &req) {
    std::tie(req.characters, req.magics, req.equips) = nibashared::staticdata::get().all();
    req.success = true;
}

void nibaserver::server_processor::process(nibashared::message_fight &req) {
    auto [self_fightable, enemy_fightable] = nibashared::prep_fight(session_, req.enemyid);
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
    if (db_.create_char(*(session_.userid), req.player, yield_)) {
        req.success = true;
        // nothing to modify for player
        session_.state = nibashared::gamestate::ingame;
        session_.player = req.player;
        BOOST_LOG_SEV(logger_, sev::info)
            << "User " << *(session_.userid) << " created an character " << req.player;
        // grab magic&equips, might not be the best way but anyway
        std::tie(session_.magics, session_.equips) = db_.get_aux(req.player.name, yield_);
        req.magics = session_.magics;
        req.equips = session_.equips;
    } else {
        req.success = false;
    }
}

const nibashared::sessionstate &nibaserver::server_processor::get_session() { return session_; }
