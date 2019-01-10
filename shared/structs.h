// this file is generated, see structs.yaml and gen_structs.py

#pragma once

#include "global_defs.h"
#include <nlohmann/json.hpp>

namespace nibashared {

using magic_ids = std::vector<int>;
using equipment_ids = std::vector<int>;

struct battlestats {
    int hp{0};
    int mp{0};
    int attack_min{0};
    int attack_max{0};
    int inner_power{0};
    int accuracy{0};
    int evasion{0};
    int speed{0};
    int defence{0};
    int crit_chance{0};
    int crit_damage{0};
    int reduce_def{0};
    int reduce_def_perc{0};
    int hp_regen{0};
    int mp_regen{0};
    int gold_res{0};
    int wood_res{0};
    int water_res{0};
    int fire_res{0};
    int earth_res{0};
    int hp_on_hit{0};
    int hp_steal{0};
    int mp_on_hit{0};
    int mp_steal{0};

    battlestats &operator+=(const battlestats &rhs) {
        hp += rhs.hp;
        mp += rhs.mp;
        attack_min += rhs.attack_min;
        attack_max += rhs.attack_max;
        inner_power += rhs.inner_power;
        accuracy += rhs.accuracy;
        evasion += rhs.evasion;
        speed += rhs.speed;
        defence += rhs.defence;
        crit_chance += rhs.crit_chance;
        crit_damage += rhs.crit_damage;
        reduce_def += rhs.reduce_def;
        reduce_def_perc += rhs.reduce_def_perc;
        hp_regen += rhs.hp_regen;
        mp_regen += rhs.mp_regen;
        gold_res += rhs.gold_res;
        wood_res += rhs.wood_res;
        water_res += rhs.water_res;
        fire_res += rhs.fire_res;
        earth_res += rhs.earth_res;
        hp_on_hit += rhs.hp_on_hit;
        hp_steal += rhs.hp_steal;
        mp_on_hit += rhs.mp_on_hit;
        mp_steal += rhs.mp_steal;
        return *this;
    }
};
void to_json(nlohmann::json &j, const battlestats &battlestats);
void from_json(const nlohmann::json &j, battlestats &battlestats);

struct attributes {
    int strength{0};
    int dexterity{0};
    int physique{0};
    int spirit{0};
};
void to_json(nlohmann::json &j, const attributes &attributes);
void from_json(const nlohmann::json &j, attributes &attributes);

struct character {
    std::string name;
    int character_id;
    attributes attrs;
    battlestats stats;
    equipment_ids equipments;
    magic_ids active_magic;
};
void to_json(nlohmann::json &j, const character &character);
void from_json(const nlohmann::json &j, character &character);

struct magic {
    std::string name;
    int magic_id;
    int active;
    int cd;
    int multiplier;
    int inner_damage;
    int mp_cost;
    property inner_property;
    battlestats stats{};
};
void to_json(nlohmann::json &j, const magic &magic);
void from_json(const nlohmann::json &j, magic &magic);

struct equipment {
    int equipment_id;
    std::string name;
    std::string description;
    equipmenttype type;
    battlestats stats{};
    int item_level;
    int required_level;
};
void to_json(nlohmann::json &j, const equipment &equipment);
void from_json(const nlohmann::json &j, equipment &equipment);

} // namespace nibashared