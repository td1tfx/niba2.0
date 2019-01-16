// this file is generated, see structs.yaml and gen_structs.py

#pragma once

#include "global_defs.h"

#include <boost/hana.hpp>
#include <nlohmann/json.hpp>

namespace nibashared {

using magic_ids = std::vector<int>;
using equipment_ids = std::vector<int>;

struct battlestats {
    BOOST_HANA_DEFINE_STRUCT(battlestats, (int, hp), (int, mp), (int, attack_min),
                             (int, attack_max), (int, inner_power), (int, accuracy), (int, evasion),
                             (int, speed), (int, defence), (int, crit_chance), (int, crit_damage),
                             (int, reduce_def), (int, reduce_def_perc), (int, hp_regen),
                             (int, mp_regen), (int, gold_res), (int, wood_res), (int, water_res),
                             (int, fire_res), (int, earth_res), (int, hp_on_hit), (int, hp_steal),
                             (int, mp_on_hit), (int, mp_steal));

    // TODO: fix this with hana
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

struct attributes {
    BOOST_HANA_DEFINE_STRUCT(attributes, (int, strength), (int, dexterity), (int, physique),
                             (int, spirit));
};

struct character {
    BOOST_HANA_DEFINE_STRUCT(character, (std::string, name), (int, character_id),
                             (attributes, attrs), (battlestats, stats), (equipment_ids, equipments),
                             (magic_ids, active_magic));
};

struct magic {
    BOOST_HANA_DEFINE_STRUCT(magic, (std::string, name), (int, magic_id), (int, active), (int, cd),
                             (int, multiplier), (int, inner_damage), (int, mp_cost),
                             (property, inner_property), (battlestats, stats));
};

struct equipment {
    BOOST_HANA_DEFINE_STRUCT(equipment, (int, equipment_id), (std::string, name),
                             (std::string, description), (equipmenttype, type),
                             (battlestats, stats), (int, item_level), (int, required_level));
};

// can't use template due to "conflict" with the json lib
void to_json(nlohmann::json &j, const battlestats &t) {
    boost::hana::for_each(t, boost::hana::fuse([&j](auto name, auto member) {
                              j[boost::hana::to<char const *>(name)] = member;
                          }));
}

void from_json(const nlohmann::json &j, battlestats &t) {
    boost::hana::for_each(boost::hana::keys(t), [&](auto key) {
        auto &member = boost::hana::at_key(t, key);
        j.at(boost::hana::to<char const *>(key)).get_to(member);
    });
}

void to_json(nlohmann::json &j, const attributes &t) {
    boost::hana::for_each(t, boost::hana::fuse([&j](auto name, auto member) {
                              j[boost::hana::to<char const *>(name)] = member;
                          }));
}

void from_json(const nlohmann::json &j, attributes &t) {
    boost::hana::for_each(boost::hana::keys(t), [&](auto key) {
        auto &member = boost::hana::at_key(t, key);
        j.at(boost::hana::to<char const *>(key)).get_to(member);
    });
}

void to_json(nlohmann::json &j, const character &t) {
    boost::hana::for_each(t, boost::hana::fuse([&j](auto name, auto member) {
                              j[boost::hana::to<char const *>(name)] = member;
                          }));
}

void from_json(const nlohmann::json &j, character &t) {
    boost::hana::for_each(boost::hana::keys(t), [&](auto key) {
        auto &member = boost::hana::at_key(t, key);
        j.at(boost::hana::to<char const *>(key)).get_to(member);
    });
}

void to_json(nlohmann::json &j, const magic &t) {
    boost::hana::for_each(t, boost::hana::fuse([&j](auto name, auto member) {
                              j[boost::hana::to<char const *>(name)] = member;
                          }));
}

void from_json(const nlohmann::json &j, magic &t) {
    boost::hana::for_each(boost::hana::keys(t), [&](auto key) {
        auto &member = boost::hana::at_key(t, key);
        j.at(boost::hana::to<char const *>(key)).get_to(member);
    });
}

void to_json(nlohmann::json &j, const equipment &t) {
    boost::hana::for_each(t, boost::hana::fuse([&j](auto name, auto member) {
                              j[boost::hana::to<char const *>(name)] = member;
                          }));
}

void from_json(const nlohmann::json &j, equipment &t) {
    boost::hana::for_each(boost::hana::keys(t), [&](auto key) {
        auto &member = boost::hana::at_key(t, key);
        j.at(boost::hana::to<char const *>(key)).get_to(member);
    });
}

} // namespace nibashared