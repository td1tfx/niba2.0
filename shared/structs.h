#pragma once
#include <array>
#include <nlohmann/json.hpp>

namespace nibashared {

enum class property { gold = 0, wood = 1, water = 2, fire = 3, earth = 4 };
enum class equipmenttype {
    head = 0,
    armor = 1,
    boots = 2,
    gloves = 3,
    belt = 4,
    amulet = 5,
    ring = 6,
    weapon = 7
};

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

    // this is inlined
    battlestats &operator+=(const battlestats &rhs) {
        attack_min += rhs.attack_min;
        attack_max += rhs.attack_max;
        accuracy += rhs.accuracy;
        evasion += rhs.evasion;
        speed += rhs.speed;
        hp += rhs.hp;
        mp += rhs.mp;
        inner_power += rhs.inner_power;
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

        return *this;
    }
};
void to_json(nlohmann::json &j, const battlestats &stats);
void from_json(const nlohmann::json &j, battlestats &stats);

struct attributes {
    int strength{0};
    int dexterity{0};
    int physique{0};
    int spirit{0};
};
void to_json(nlohmann::json &j, const attributes &attr);
void from_json(const nlohmann::json &j, attributes &attr);

using magic_ids = std::vector<int>;
using equipment_ids = std::vector<int>;
// a character
struct character {
    std::string name;
    int character_id; // definitely unique

    // attributes
    attributes attrs;
    // also allow stats
    battlestats stats;

    // since we will be passing this around
    // it's probably a better idea to use ids
    equipment_ids equipments;
    magic_ids active_magic;
};
void to_json(nlohmann::json &j, const character &character);
void from_json(const nlohmann::json &j, character &character);

struct magic {
    std::string name;
    int id;

    int active; // has CD?
    int cd;
    int multiplier;   // in percentage
    int inner_damage; // certain value
    int mp_cost;
    property inner_property;

    battlestats stats{};
};
void to_json(nlohmann::json &j, const magic &magic);
void from_json(const nlohmann::json &j, magic &magic);

const static magic DEFAULT_MAGIC{.name = u8"default",
                                 .id = 0,
                                 .active = 1,
                                 .cd = 0,
                                 .multiplier = 100,
                                 .inner_damage = 0,
                                 .mp_cost = 0,
                                 .inner_property = property::gold,
                                 .stats = {}};

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
