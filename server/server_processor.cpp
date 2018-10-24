#include "server_processor.h"
#include "db_accessor.h"
#include "fight.h"
#include "gamedata.h"
#include "rng.h"

#include <iostream>

using namespace nibaserver;

server_processor::server_processor() { session.state = nibashared::gamestate::prelogin; }

void nibaserver::server_processor::process(nibashared::message_register &req) {
    if (db_accessor::create_user(req.id, req.password)) {
        req.success = true;
        logger.log(str(boost::format("User %1% has registered.") % req.id));
    } else {
        req.success = false;
        logger.log(str(boost::format("User %1% failed to register.") % req.id), logging::trivial::warning);
    }
}

void nibaserver::server_processor::process(nibashared::message_login &req) {
    if (db_accessor::login(req.id, req.password)) {
        req.success = true;
        session.state = nibashared::gamestate::selectchar;
        session.userid = req.id;
        req.characters = {"niba1", "niba2"};
    } else {
        req.success = false;
    }
    // TODO change this later
    session.charid = 0;
    logger.log(str(boost::format("User %1% logged in.") % req.id));
}

void nibaserver::server_processor::process(nibashared::message_fight &req) {
    auto [self_fightable, enemy_fightable] = nibashared::prep_fight(session.charid, req.enemyid);
    nibashared::fight fight(std::move(self_fightable), std::move(enemy_fightable));
    nibashared::rng_server rng;
    fight.go(rng);
    req.generated = std::move(rng.generated);
}

const nibashared::sessionstate &nibaserver::server_processor::get_session() { return session; }
