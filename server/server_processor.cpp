#include "server_processor.h"
#include "db_accessor.h"
#include "fight.h"
#include "gamedata.h"
#include "rng.h"

#include <iostream>

using namespace nibaserver;
namespace sev = boost::log::trivial;

server_processor::server_processor() { session_.state = nibashared::gamestate::prelogin; }

void nibaserver::server_processor::process(nibashared::message_register &req) {
    if (db_accessor::create_user(req.id, req.password)) {
        req.success = true;
        BOOST_LOG_SEV(logger_, sev::info) << "User " << req.id << " has registered.";
    } else {
        req.success = false;
        BOOST_LOG_SEV(logger_, sev::warning) << "User " << req.id << " failed to register.";
    }
}

void nibaserver::server_processor::process(nibashared::message_login &req) {
    if (db_accessor::login(req.id, req.password)) {
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
    auto [self_fightable, enemy_fightable] = nibashared::prep_fight(5, 7);
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
    if (db_accessor::create_char(*(session_.userid), std::move(c))) {
        req.success = true;
        session_.state = nibashared::gamestate::ingame;
        BOOST_LOG(logger_) << "User " << *(session_.userid) << " created an character " << req.name;
    } else {
        req.success = false;
    }
}

const nibashared::sessionstate &nibaserver::server_processor::get_session() { return session_; }
