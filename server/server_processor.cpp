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
        session_.state = nibashared::gamestate::selectchar;
        session_.userid = req.id;
        req.characters = {"niba1", "niba2"};
    } else {
        req.success = false;
    }
    // TODO change this later
    session_.charid = 0;
    BOOST_LOG_SEV(logger_, sev::info) << "User " << req.id << " logged in.";
}

void nibaserver::server_processor::process(nibashared::message_fight &req) {
    auto [self_fightable, enemy_fightable] = nibashared::prep_fight(session_.charid, req.enemyid);
    nibashared::fight fight(std::move(self_fightable), std::move(enemy_fightable));
    nibashared::rng_server rng;
    fight.go(rng);
    req.generated = std::move(rng.generated);
}

const nibashared::sessionstate &nibaserver::server_processor::get_session() { return session_; }
