#pragma once
#include "global_defs.h"
#include "rng.h"
#include "structs.h"

#include <optional>
#include <utility>
#include <vector>

namespace nibashared {

struct magic_ex {
    int cd{0};
    magic real_magic{};

    void heatup();
    void cooldown(int ticks);
};

const static magic DEFAULT_MAGIC{.name = u8"default",
                                 .magic_id = 0,
                                 .static_id = -1,
                                 .active = 1,
                                 .cd = 0,
                                 .multiplier = 100,
                                 .inner_damage = 0,
                                 .mp_cost = 0,
                                 .inner_property = property::gold,
                                 .stats = {}};

constexpr int FIGHT_MAX_PROG = 1000;
constexpr double DEFENCE_EXTENSION = 100;
constexpr double INNER_BASE = 100;

using fightable_magics = std::vector<magic_ex>;

class fightable {
public:
    character char_data; // stats unchanged
    std::size_t idx;     // idx within all_
    int team;            // takes value 0 or 1
    int progress{0};

    // Note here the person may not have full 5 magics
    fightable_magics magics;

    int ticks_calc() const;
    template<typename Rng>
    int damage_calc(const magic &chosen_magic, const fightable &defender, Rng &rng) const;
    int threat_calc() const;

    // note that return also makes sure the real magic is there if has value
    std::optional<std::size_t> pick_magic_idx();
};

class fight {
public:
    // the return is the RNG results! this is to sync with the client
    // 1. it's hard to set random seed to get the same randoms for different platform of clients
    // 2. so instead of returning damage numbers, returning randoms
    fight(std::vector<fightable> &&friends, std::vector<fightable> &&enemies);

    // RNG will be used to provide random numbers (from 0-1 likely)
    // the test client will have the set of numbers received from server
    template<typename Rng>
    int go(Rng &rng);

private:
    std::size_t friends_;
    std::size_t enemies_;
    std::vector<fightable> all_;
};

battlestats stats_computer(const attributes &attr);

template<typename staticdata>
fightable setup_fightable(int id) {
    auto raw_character = staticdata::get().character(id);
    fightable fight_character;
    fight_character.char_data = raw_character;
    fight_character.char_data.stats += stats_computer(raw_character.attrs);
    for (auto magic_id : raw_character.active_magic) {
        auto magic = staticdata::get().magic(magic_id);
        CPRINT(raw_character.name << " has magic " << magic.name);
        fight_character.char_data.stats += magic.stats;
        // NOTE, aggregation initialization is copy... well fine
        // also push_back is complaining designated initialization for reasons I don't understand
        fight_character.magics.emplace_back(magic_ex{.cd = 0, .real_magic = magic});
    }
    for (auto equip_id : raw_character.equipments) {
        auto equipment = staticdata::get().equipment(equip_id);
        CPRINT(raw_character.name << " has item " << equipment.name);
        fight_character.char_data.stats += equipment.stats;
    }
    return fight_character; // RVO
}

template<typename staticdata>
std::pair<std::vector<fightable>, std::vector<fightable>> prep_fight(int id_me, int id_you) {
    CPRINT("prep " << id_me << " " << id_you);
    // initializer list do not like moves
    std::pair<std::vector<fightable>, std::vector<fightable>> ret;
    ret.first.push_back(setup_fightable<staticdata>(id_me));
    ret.second.push_back(setup_fightable<staticdata>(id_you));
    return ret; // RVO
}

} // namespace nibashared
