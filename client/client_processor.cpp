#include "client_processor.h"
#include "fight.h"
#include "gamedata.h"
#include "rng.h"
#include "util.h"

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
            for (auto &magic : session_.magics) {
                magic_static_id_map[magic.static_id] = magic.magic_id;
            }
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
        for (auto &magic : session_.magics) {
            magic_static_id_map[magic.static_id] = magic.magic_id;
        }
    } else {
        std::cout << "unable to create character" << std::endl;
    }
}

void nibaclient::client_processor::process(nibashared::message_learnmagic &req) {
    if (req.success) {
        std::cout << "learned magic " << req.magic.name << std::endl;
        magic_static_id_map[req.magic.static_id] = req.magic.magic_id;
        session_.magics.push_back(std::move(req.magic));
    } else {
        std::cout << "failed to learn magic" << std::endl;
    }
}

void nibaclient::client_processor::process(nibashared::message_fusemagic &req) {
    if (req.success) {
        // remove secondary magic and update primary magic
        nibautil::vector_remove(session_.magics, [&req](auto &magic) {
            return magic.magic_id == req.secondary_magic_id;
        });
        // remove secondary if in equipped magics
        nibautil::vector_remove(session_.equipped_magic_ids, [&req](auto &magic_id) {
            return magic_id == req.secondary_magic_id;
        });
        auto primary_magic_iter = nibautil::find_if(session_.magics, [&req](auto &magic) {
            return magic.magic_id == req.primary_magic_id;
        });
        *primary_magic_iter = req.magic;
        std::cout << "fused magic: " << req.magic << std::endl;
    } else {
        std::cout << "failed to fuse magic" << std::endl;
    }
}

void nibaclient::client_processor::process(nibashared::message_reordermagic &req) {
    if (req.success) {
        session_.equipped_magic_ids = std::move(req.equipped_magic_ids);
    } else {
        std::cout << "failed to reorder magic" << std::endl;
    }
}

const nibashared::sessionstate &client_processor::get_session() { return session_; }
