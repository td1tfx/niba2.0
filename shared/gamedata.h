#pragma once
#include "global_defs.h"
#include <array>
#include <boost/assert.hpp>
#include <optional>
#include <vector>

// this is temporary

namespace nibashared {

namespace temporary {

character character_list[]{
    {u8"泥巴", 0, {10, 10, 10, 10}, {10, 10, 10, 10, 10, 120, 10, 10}, {0, -1, -1, -1, -1}},
    {u8"机器猫", 1, {10, 10, 10, 10}, {10, 10, 10, 10, 10, 100, 10, 10}, {1, 2, 3, -1, -1}}};

/*
    std::string_view name;
    int id;

    int active; // has CD?
    int cd;
    int multiplier; // in percentage

    int attack_min{0};
    int attack_max{0};
    int accuracy{0};
    int evasion{0};
    int speed{0};
    int hp{0};
    int mp{0};
    int inner_power{0};

*/
magic magic_list[]{{u8"m1", 0, 1, 500, 240, 100, {}},
                   {u8"m2", 0, 0, 100, 100, 100, {2, 2, 5, 5, 2, 10, 10, 10, 10}},
                   {u8"m3", 0, 1, 120, 100, 100, {}},
                   {u8"m4", 0, 1, 110, 100, 100, {}},
                   {u8"m5", 0, 1, 100, 100, 100, {}}};

} // namespace temporary

character get_character(int id) { return temporary::character_list[id]; }

fightable_magics get_magics(const character_magic_ids &ids) {
    fightable_magics magics;
    for (std::size_t i = 0; i < magics.size(); i++) {
        if (ids[i] >= 0) {
            magics[i].real_magic = temporary::magic_list[ids[i]];
        } else {
            magics[i].real_magic = std::nullopt;
        }
    }
    return magics;
}

// this is temporary too
std::pair<std::vector<fightable>, std::vector<fightable>> prep_fight(int id_me, int id_you) {
    // refactor this
    auto self = nibashared::get_character(id_me);
    auto you = nibashared::get_character(id_you);

    std::vector<nibashared::fightable> self_fightable{{}};
    std::vector<nibashared::fightable> enemy_fightable{{}};
    self_fightable.back().stats = self.stats;
    enemy_fightable.back().stats = you.stats;
    self_fightable.back().magics = nibashared::get_magics(self.active_magic);
    enemy_fightable.back().magics = nibashared::get_magics(you.active_magic);

    return {self_fightable, enemy_fightable};
}

} // namespace nibashared