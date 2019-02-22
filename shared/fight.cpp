#include "fight.h"
#include "gamedata.h"
#include "rng.h"
#include "util.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <iostream>

namespace nibashared {

void magic_ex::heatup() { cd = real_magic.cd; }
void magic_ex::cooldown(int ticks) { cd = std::max(cd - ticks, 0); }

int fightable::ticks_calc() const {
    if (progress >= FIGHT_MAX_PROG)
        return 0;
    // this rounds up
    // for example, the current progress is 649, speed is 35
    // (1000 - 649 + 35 - 1) / 35 = 11 ticks to finish the 351 progress
    // say the progress is now 651, speed is still 35
    // (1000 - 651 + 35 - 1) / 35 = 10 ticks to finish the 349 progress
    return (FIGHT_MAX_PROG - progress + char_data.stats.speed - 1) / char_data.stats.speed;
}

template<typename Rng>
int fightable::damage_calc(const magic &chosen_magic, const fightable &defender, Rng &rng) const {
    int accuracy =
        static_cast<int>(char_data.stats.accuracy *
                         (100.0 / (char_data.stats.accuracy + defender.char_data.stats.accuracy)));
    if (accuracy <= rng(0, 99)) {
        CPRINT("the attack was evaded!");
        return 0;
    }

    int physical_damage = rng(char_data.stats.attack_min, char_data.stats.attack_max);
    double physical_damage_multiplier = chosen_magic.multiplier / 100.0;
    double physical_damage_reduction =
        defender.char_data.stats.defence / (defender.char_data.stats.defence + DEFENCE_EXTENSION);

    int inner_damage = chosen_magic.inner_damage;
    double inner_damage_multiplier = 1 + char_data.stats.inner_power / INNER_BASE;
    double inner_damage_reduction;
    switch (chosen_magic.inner_property) {
    case property::gold:
        inner_damage_reduction = defender.char_data.stats.gold_res / 100.0;
        break;
    case property::wood:
        inner_damage_reduction = defender.char_data.stats.wood_res / 100.0;
        break;
    case property::water:
        inner_damage_reduction = defender.char_data.stats.water_res / 100.0;
        break;
    case property::fire:
        inner_damage_reduction = defender.char_data.stats.fire_res / 100.0;
        break;
    case property::earth:
        inner_damage_reduction = defender.char_data.stats.earth_res / 100.0;
        break;
    default:
        inner_damage_reduction = 1.0;
        break;
    }
    CPRINT("the attack landed!")
    CPRINT("physical " << physical_damage << " " << physical_damage_multiplier << " "
                       << physical_damage_reduction);
    CPRINT("inner " << inner_damage << " " << inner_damage_multiplier << " "
                    << inner_damage_reduction);
    return static_cast<int>(physical_damage * physical_damage_multiplier *
                                (1 - physical_damage_reduction) +
                            inner_damage * inner_damage_multiplier * (1 - inner_damage_reduction));
}

template int fightable::damage_calc<rng_client>(const magic &, const fightable &,
                                                rng_client &) const;
template int fightable::damage_calc<rng_server>(const magic &, const fightable &,
                                                rng_server &) const;

int fightable::threat_calc() const {
    return char_data.stats.attack_min + char_data.stats.attack_max;
}

// note that return also makes sure the real magic is there if has value
std::optional<std::size_t> fightable::pick_magic_idx() {
    // potential optimization: precompute the sequence of magics
    for (std::size_t i = 0; i < magics.size(); i++) {
        if (magics[i].cd == 0 && magics[i].real_magic.mp_cost < char_data.stats.mp) {
            return i;
        }
    }
    return {};
}

fight::fight(std::vector<fightable> &&friends, std::vector<fightable> &&enemies) {
    friends_ = friends.size();
    enemies_ = enemies.size();
    nibautil::for_each(friends, [](auto &f) { f.team = 0; });
    nibautil::for_each(enemies, [](auto &f) { f.team = 1; });
    std::move(friends.begin(), friends.end(), std::back_inserter(all_));
    std::move(enemies.begin(), enemies.end(), std::back_inserter(all_));
    for (std::size_t i = 0; i < all_.size(); i++) {
        all_.at(i).idx = i;
    }
    // sort by speed, use the 'arbitrary' idx to break tie
    std::sort(all_.begin(), all_.end(), [](const auto &lhs, const auto &rhs) {
        return std::tie(lhs.char_data.stats.speed, lhs.idx) <
               std::tie(rhs.char_data.stats.speed, rhs.idx);
    });
    for (std::size_t i = 0; i < all_.size(); i++) {
        all_.at(i).idx = i;
    }
}

template<typename Rng>
int fight::go(Rng &rng) {
    CPRINT("fight begins" << std::endl);
    std::size_t team_alive_count[]{friends_, enemies_};
    std::vector<std::size_t> alive;
    alive.reserve(all_.size());
    for (;;) {
        if (team_alive_count[0] == 0 || team_alive_count[1] == 0)
            break;
        // all alive indices
        for (std::size_t i = 0; i < all_.size(); i++) {
            if (all_.at(i).char_data.stats.hp <= 0) {
                continue;
            }
            alive.push_back(i);
        }

        using tick_idx_p = std::pair<int, std::size_t>;
        // find the person that is the fastest to act
        tick_idx_p min_tick{all_.at(alive[0]).ticks_calc(), alive[0]};
        // figure out the ticks to make it to the next action
        for (std::size_t i = 1; i < alive.size(); i++) {
            tick_idx_p tick{all_.at(alive[i]).ticks_calc(), alive[i]};
            if (tick < min_tick)
                min_tick = tick;
        }

        auto &[ticks, attacker_idx] = min_tick;
        auto &attacker = all_.at(attacker_idx);

        // make everyone progress and find the target, to break ties, again use idx
        using threat_idx_p = std::pair<int, std::size_t>;
        // it's unknown which idx is the first enemy
        // we use (threat=-1, idx=attacker) as a placeholder
        threat_idx_p max_threat{-1, attacker.idx};
        for (auto idx : alive) {
            auto &p = all_.at(idx);
            p.progress += p.char_data.stats.speed * ticks;
            // clang has some problems dealing with lambdas and structured bindings
            for (auto &mex : p.magics) {
                mex.cooldown(ticks);
            }
            // maybe optimization: maintain a priority list (heap) of top threats
            if (p.team != attacker.team) {
                threat_idx_p threat{p.threat_calc(), idx};
                if (threat > max_threat)
                    max_threat = threat;
            }
        }
        BOOST_ASSERT(max_threat.second != attacker.idx);
        auto &defender = all_.at(max_threat.second);

        // now pick a magic to use
        auto chosen_magic_idx = attacker.pick_magic_idx();

        // use just puts it on cd, extra + 1
        auto dmg = 0;
        if (chosen_magic_idx) {
            auto &chosen_magicex = attacker.magics[(*chosen_magic_idx)];
            chosen_magicex.heatup();
            dmg = attacker.damage_calc(chosen_magicex.real_magic, defender, rng);
            attacker.char_data.stats.mp -= chosen_magicex.real_magic.mp_cost;
            CPRINT(attacker.char_data.name << " uses magic " << *chosen_magic_idx
                                           << chosen_magicex.real_magic.name << " dealt " << dmg
                                           << " on " << defender.char_data.name);
        } else {
            dmg = attacker.damage_calc(DEFAULT_MAGIC, defender, rng);
            CPRINT(attacker.char_data.name << " uses magic " << DEFAULT_MAGIC.name << " dealt "
                                           << dmg << " on " << defender.char_data.name);
        }

        defender.char_data.stats.hp -= dmg;

        CPRINT(defender.char_data.name << " hp remains " << defender.char_data.stats.hp
                                       << std::endl);

        if (defender.char_data.stats.hp <= 0) {
            team_alive_count[defender.team] -= 1;
        }
        attacker.progress = 0;
        alive.clear();
    }
    CPRINT(team_alive_count[0] << " " << team_alive_count[1]);
    return team_alive_count[1] == 0 ? 0 : 1;
}

template int fight::go<rng_client>(rng_client &rng);
template int fight::go<rng_server>(rng_server &rng);

// will move this elsewhere
battlestats stats_computer(const attributes &attr) {
    battlestats stats{};
    stats.attack_min = attr.strength * 2;
    stats.attack_max = attr.strength * 2;
    stats.evasion = attr.dexterity;
    stats.speed = attr.dexterity;
    stats.accuracy = attr.dexterity;
    stats.hp = 5 * attr.physique;
    stats.defence = attr.physique;
    stats.mp = 5 * attr.spirit;
    stats.inner_power = attr.spirit;
    return stats; // RVO
}

fightable setup_self(nibashared::character &&raw_character,
                     const std::vector<nibashared::magic> &magics,
                     const std::vector<nibashared::equipment> &equips,
                     const std::vector<int> &magic_ids) {
    fightable fight_character;
    fight_character.char_data = std::move(raw_character);
    fight_character.char_data.stats += stats_computer(fight_character.char_data.attrs);
    for (const auto &magic : magics) {
        if (std::find(magic_ids.begin(), magic_ids.end(), magic.static_id) == magic_ids.end()) {
            continue;
        }
        CPRINT(fight_character.char_data.name << " equipped magic " << magic.name);
        fight_character.char_data.stats += magic.stats;
        // NOTE, aggregation initialization is copy... well fine
        // also push_back is complaining designated initialization for reasons I don't understand
        fight_character.magics.emplace_back(magic_ex{.cd = 0, .real_magic = magic});
    }
    for (const auto &equipment : equips) {
        CPRINT(fight_character.char_data.name << " has item " << equipment.name);
        fight_character.char_data.stats += equipment.stats;
    }
    return fight_character; // RVO
}

fightable setup_fightable(int id) {
    nibashared::character raw_character = staticdata::get().character(id);
    std::vector<nibashared::magic> magics;
    std::vector<nibashared::equipment> equips;
    std::vector<int> magic_ids;
    nibautil::for_each(raw_character.active_magic, [&magics, &magic_ids](auto &magic_id) {
        magic_ids.push_back(magic_id);
        magics.push_back(staticdata::get().magic(magic_id));
    });
    nibautil::for_each(raw_character.equipments, [&equips](auto &equip_id) {
        equips.push_back(staticdata::get().equipment(equip_id));
    });
    // std::cout << magics << std::endl;
    // std::cout << equips << std::endl;
    return setup_self(std::move(raw_character), magics, equips, magic_ids);
}

std::pair<std::vector<fightable>, std::vector<fightable>>
prep_fight(nibashared::sessionstate &session, int id_you) {
    CPRINT("prep " << (*(session.player)).name << " " << id_you);
    std::pair<std::vector<fightable>, std::vector<fightable>> ret;
    auto &magic_ids = session.equipped_magic_ids;
    // copy player data into raw_character
    nibashared::character raw_character{.name = (*(session.player)).name,
                                        .character_id = -1,
                                        .description = {},
                                        .attrs = (*(session.player)).attrs,
                                        // leave these 3 blank
                                        .stats = {},
                                        .equipments = {},
                                        .active_magic = {}};
    ret.first.push_back(
        setup_self(std::move(raw_character), session.magics, session.equips, magic_ids));
    ret.second.push_back(setup_fightable(id_you));
    return ret; // RVO
}

} // namespace nibashared