#include "server_processor.h"
#include "calcdelay.h"
#include "db_accessor.h"
#include "fight.h"
#include "gamedata.h"
#include "rng.h"
#include "util.h"

#include <array>
#include <iostream>

using namespace nibaserver;
namespace sev = boost::log::trivial;

server_processor::server_processor(boost::asio::yield_context &yield, db_accessor &db,
                                   session_map &ss_map, session_wptr ss_wptr) :
    session_{},
    yield_{yield}, db_{db}, ss_map_{ss_map}, ss_wptr_{ss_wptr} {}

std::string server_processor::dispatch(const std::string &request) {
    // logging the request is bad, when password is involved
    // BOOST_LOG_SEV(logger_, sev::debug) << request;
    try {
        session_.current_time = std::chrono::high_resolution_clock::now();
        if (session_.current_time < session_.earliest_time) {
            throw std::runtime_error("request too frequent");
        }
        session_.earliest_time = session_.current_time;

        auto j = nlohmann::json::parse(request);
        return nibashared::message::dispatcher(j, [this](auto req) {
            if (!req.base_validate(session_)) {
                throw std::runtime_error("validation failure");
            }
            process(req);
            return req.base_create_response().dump();
        });
    }
    // return whatever error message, I don't care
    catch (std::exception &e) {
        BOOST_LOG_SEV(logger_, sev::info) << "dispatch failure: " << e.what();
    }
    nlohmann::json error_msg{{"error", "request error"}};
    return error_msg.dump();
}

void nibaserver::server_processor::process(nibashared::message_registration &req) {
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
        if (req.player && ss_map_.register_session((*(req.player)).name, ss_wptr_)) {
            session_.state = nibashared::gamestate::ingame;
            session_.player = *(req.player);
            // player data exists, so lets fetch equipments&magics data of the player
            session_.data = db_.get_aux((*(req.player)).name, yield_);
            req.data = session_.data;
            BOOST_LOG_SEV(logger_, sev::info)
                << "User " << *(session_.userid) << " data " << *(req.data);
        } else {
            session_.state = nibashared::gamestate::createchar;
        }
    } else {
        req.success = false;
    }
    BOOST_LOG_SEV(logger_, sev::info) << "User " << req.id << " logging in is " << req.success;
}

void nibaserver::server_processor::process(nibashared::message_getdata &req) {
    nibashared::getdata().to_map(req.characters);
    nibashared::getdata().to_map(req.magics);
    nibashared::getdata().to_map(req.equips);
    nibashared::getdata().to_map(req.maps);
    req.success = true;
}

void nibaserver::server_processor::process(nibashared::message_fight &req) {
    // TODO assume fixed map id of 1, figure out the enemy
    auto [self_fightable, enemy_fightable] = nibashared::prep_fight(session_, req.enemyid);
    // Maybe refactor again..
    auto max_hp = static_cast<double>(self_fightable.at(0).char_data.stats.hp);
    nibashared::fight fight(std::move(self_fightable), std::move(enemy_fightable));
    nibashared::rng_server rng;
    fight.go(rng);
    req.generated = std::move(rng.generated);
    session_.earliest_time += nibashared::fight_delay(max_hp, fight.my_status().char_data.stats.hp,
                                                      fight.elapsed_ticks());
}

void nibaserver::server_processor::process(nibashared::message_createchar &req) {
    if (!session_.userid) {
        req.success = false;
        return;
    }
    // players have an id of -1? or auto increment?
    if (db_.create_char(*(session_.userid), req.player, yield_) &&
        ss_map_.register_session(req.player.name, ss_wptr_)) {
        req.success = true;
        // nothing to modify for player
        session_.state = nibashared::gamestate::ingame;
        session_.player = req.player;
        BOOST_LOG_SEV(logger_, sev::info)
            << "User " << *(session_.userid) << " created an character " << req.player;
        session_.data = db_.get_aux(req.player.name, yield_);
        req.data = session_.data;
        BOOST_LOG_SEV(logger_, sev::info) << "User " << *(session_.userid) << " data " << req.data;
    } else {
        req.success = false;
    }
}

void nibaserver::server_processor::process(nibashared::message_learnmagic &req) {
    nibashared::staticdata::get().to(req.static_id, req.magic);
    if (req.magic.description.empty()) {
        req.magic.description = req.magic.name;
    }
    // by default we add it to the back
    auto result = db_.create_magic(session_.player.value().name, req.magic, yield_);
    if (!(req.success = result)) {
        return;
    }
    session_.data.magics.push_back(req.magic);
}

void nibaserver::server_processor::process(nibashared::message_fusemagic &req) {
    // this is a double find, but it's what it is
    // validate has to be separated out, we could cache more things in memory
    auto primary_magic_iter = nibautil::find_if(session_.data.magics, [&req](auto &magic) {
        return magic.magic_id == req.primary_magic_id;
    });
    // TODO maybe refactor huge chunk of logic out of here
    req.magic = *primary_magic_iter;
    for (auto &magic : session_.data.magics) {
        if (magic.magic_id == req.secondary_magic_id) {
            auto secondary_stats = magic.stats;
            // for most of the stats, do 5%
            secondary_stats *= 0.05;
            // pick the 20% stat based on player main attributes
            auto &attrs = session_.player->attrs;
            auto selected = nibashared::weighted_rand(attrs.get_array_ref());
            using selector = nibashared::attributes::selector;
            selector attr_enum{selected};
            auto extra_stats = [PRIM_MULT = 0.15](int &primary, int secondary) {
                primary += std::lround(secondary * PRIM_MULT);
            };
            BOOST_LOG_SEV(logger_, sev::info) << "magic selector index " << selected;
            switch (attr_enum) {
            // prefer attack for strength
            case selector::strength:
                extra_stats(secondary_stats.attack_min, magic.stats.attack_min);
                extra_stats(secondary_stats.attack_max, magic.stats.attack_max);
                break;
            // prefer accuracy, evasion, speed
            case selector::dexterity:
                extra_stats(secondary_stats.accuracy, magic.stats.accuracy);
                extra_stats(secondary_stats.evasion, magic.stats.evasion);
                extra_stats(secondary_stats.speed, magic.stats.speed);
                break;
            // prefer hp and def
            case selector::physique:
                extra_stats(secondary_stats.hp, magic.stats.hp);
                extra_stats(secondary_stats.defence, magic.stats.defence);
                break;
            // prefer mp and inner_power
            case selector::spirit:
                extra_stats(secondary_stats.mp, magic.stats.mp);
                extra_stats(secondary_stats.inner_power, magic.stats.inner_power);
                break;
            }

            req.magic.stats += secondary_stats;

            // inner damage and multiplier also increment a bit
            req.magic.inner_damage += std::lround(magic.inner_damage * 0.05);
            req.magic.multiplier += std::lround((magic.multiplier - 100) * 0.05);

            // figure out the new equipped magics
            std::vector<int> new_equipped;
            std::copy_if(session_.data.equipped_magic_ids.begin(),
                         session_.data.equipped_magic_ids.end(), std::back_inserter(new_equipped),
                         [removing = req.secondary_magic_id](int id) { return id != removing; });
            BOOST_LOG_SEV(logger_, sev::info) << "removing magic_id " << req.secondary_magic_id;
            BOOST_LOG_SEV(logger_, sev::info) << "new equipped ids " << req.secondary_magic_id;
            auto result = db_.fuse_magic(session_.player.value().name, req.magic,
                                         req.secondary_magic_id, new_equipped, yield_);
            if (!(req.success = result)) {
                return;
            }
            // if db is ok, update in memory data, well if there is a problem then there is a
            // problem... at least db and memory is consistently not updated
            session_.data.equipped_magic_ids = new_equipped;
            nibautil::vector_remove(session_.data.magics, [&req](auto &magic) {
                return magic.magic_id == req.secondary_magic_id;
            });
            *primary_magic_iter = req.magic;
            // note we were iterating session_.magic, must break now
            break;
        }
    }
}

void nibaserver::server_processor::process(nibashared::message_reordermagic &req) {
    // auto result = db_.
    auto result = db_.equip_magics(session_.player.value().name, req.equipped_magic_ids, yield_);
    if (!(req.success = result)) {
        return;
    }
    session_.data.equipped_magic_ids = req.equipped_magic_ids;
}

void nibaserver::server_processor::process(nibashared::message_echo &) {}

void nibaserver::server_processor::process(nibashared::message_send &req) {
    BOOST_LOG_SEV(logger_, sev::info) << session_.player.value().name << " sending message to " << req.name;
    nibashared::message_echo echo{std::move(req.message)};
    req.success = ss_map_.write(req.name, echo.base_create_request().dump());
}

const nibashared::sessionstate &nibaserver::server_processor::get_session() { return session_; }
