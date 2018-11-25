#include "client_processor.h"
#include "fight.h"
#include "gamedata.h"
#include "rng.h"

#include <iostream>

using namespace nibaclient;

client_processor::client_processor() { session.state = nibashared::gamestate::prelogin; }

void client_processor::process(nibashared::message_register &req) {
    if (req.success) {
        std::cout << "success" << std::endl;
    } else {
        std::cout << "unable to register" << std::endl;
    }
}

void client_processor::process(nibashared::message_login &req) {
    if (req.success) {
        std::cout << "success" << std::endl;
        session.userid = req.id;
        // TODO look up character db, if no char, then create one
        // just createchar for now
        session.state = nibashared::gamestate::createchar;
    } else {
        std::cout << "failed to login" << std::endl;
    }
}

void nibaclient::client_processor::process(nibashared::message_fight &req) {
    nibashared::rng_client rng(std::move(req.generated));
    auto [self_fightable, enemy_fightable] =
        nibashared::prep_fight(7, 8);
    // session.charid, req.enemyid
    nibashared::fight fight(std::move(self_fightable), std::move(enemy_fightable));
    std::cout << fight.go(rng) << " wins" << std::endl;
}

void nibaclient::client_processor::process(nibashared::message_createchar &req) {
    if (req.success) {
        std::cout << "success" << std::endl;
        session.state = nibashared::gamestate::ingame;
    } else {
        std::cout << "unable to create character" << std::endl;
    }
}

const nibashared::sessionstate &client_processor::get_session() { return session; }
