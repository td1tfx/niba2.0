#include "server_processor.h"
#include "calcdelay.h"
#include "db_accessor.h"
#include "fight.h"
#include "gamedata.h"
#include "rng.h"
#include "util.h"

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
        case nibashared::cmdtype::learnmagic: {
            return do_request<nibashared::message_learnmagic>(j);
        }
        case nibashared::cmdtype::fusemagic: {
            return do_request<nibashared::message_fusemagic>(j);
        }
        case nibashared::cmdtype::reordermagic: {
            return do_request<nibashared::message_reordermagic>(j);
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
    if (db_.create_char(*(session_.userid), req.player, yield_)) {
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
    req.magic = *primary_magic_iter;
    for (auto &magic : session_.data.magics) {
        if (magic.magic_id == req.secondary_magic_id) {
            auto secondary_stats = magic.stats;
            // TODO:
            // 根据人物属性，随机选取被融合武功的其中一项属性为融合主属性，其余为附属性。
            // （力量除以胫骨的比值越大，选取物理输出作为主属性概率越高）
            // 武功融合过程，主属性武功+附武功主属性的0%~10%，副属性+附武功属性的-1%~3%。
            // secondary_stats *= 0.1;
            req.magic.stats += secondary_stats;
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

const nibashared::sessionstate &nibaserver::server_processor::get_session() { return session_; }
