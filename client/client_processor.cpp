#include "client_processor.h"
#include "fight.h"
#include "gamedata.h"
#include "rng.h"

#include <iostream>

using namespace nibaclient;

client_processor::client_processor() : session_() {}

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
        session_.userid = req.id;
        if (!req.player) {
            session_.state = nibashared::gamestate::createchar;
        } else {
            session_.state = nibashared::gamestate::ingame;
            std::cout << *req.player << std::endl;
            session_.player = *(req.player);
            session_.magics = req.magics;
            session_.equips = req.equips;
        }
    } else {
        std::cout << "failed to login" << std::endl;
    }
}

void nibaclient::client_processor::process(nibashared::message_getdata &req) {
    if (req.success) {
        nibashared::staticdata::init([&req](auto &characters, auto &magics, auto &equipments) {
            characters = req.characters;
            magics = req.magics;
            equipments = req.equips;
        });
    } else {
        std::cout << "unable to fetch gamedata" << std::endl;
        // probably should crash
    }
}

void nibaclient::client_processor::process(nibashared::message_fight &req) {
    nibashared::rng_client rng(std::move(req.generated));
    auto [self_fightable, enemy_fightable] = nibashared::prep_fight(session_, req.enemyid);
    nibashared::fight fight(std::move(self_fightable), std::move(enemy_fightable));
    std::cout << fight.go(rng) << " wins" << std::endl;
}

void nibaclient::client_processor::process(nibashared::message_createchar &req) {
    if (req.success) {
        std::cout << "success" << std::endl;
        std::cout << req.player << std::endl;
        session_.player = req.player;
        session_.magics = req.magics;
        session_.equips = req.equips;
        session_.state = nibashared::gamestate::ingame;
    } else {
        std::cout << "unable to create character" << std::endl;
    }
}

const nibashared::sessionstate &client_processor::get_session() { return session_; }
