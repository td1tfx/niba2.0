#include "server_processor.h"
#include "db_accessor.h"
#include "fight.h"
#include "rng.h"
#include "gamedata.h"

#include <iostream>

using namespace nibaserver;

server_processor::server_processor() { session.state = nibashared::gamestate::prelogin; }

void nibaserver::server_processor::process(nibashared::message_register &req) {
    if (db_accessor::create_user(req.id, req.password)) {
        req.success = true;
    } else {
        req.success = false;
    }
    std::cout << "register " << req.success << std::endl;
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
    std::cout << "login " << req.success << std::endl;
}

void nibaserver::server_processor::process(nibashared::message_fight & req)
{
    auto [self_fightable, enemy_fightable] = nibashared::prep_fight(session.charid, req.enemyid);
    nibashared::fight fight(std::move(self_fightable), std::move(enemy_fightable));
    nibashared::rng_server rng;
    fight.go(rng);
    req.generated = std::move(rng.generated);
}

const nibashared::sessionstate &nibaserver::server_processor::get_session() { return session; }
