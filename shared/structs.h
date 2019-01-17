// this file is generated, see structs.yaml and gen_structs.py

#pragma once

#include "global_defs.h"

#include <boost/hana.hpp>
#include <nlohmann/json.hpp>

namespace nibashared {

using magic_ids = std::vector<int>;
using equipment_ids = std::vector<int>;

template<typename T>
void hana_to_json(nlohmann::json &j, const T &t) {
    boost::hana::for_each(t, boost::hana::fuse([&j](auto name, auto member) {
                              j[boost::hana::to<char const *>(name)] = member;
                          }));
}

template<typename T>
void hana_from_json(const nlohmann::json &j, T &t) {
    boost::hana::for_each(boost::hana::keys(t), [&](auto key) {
        auto &member = boost::hana::at_key(t, key);
        j.at(boost::hana::to<char const *>(key)).get_to(member);
    });
}

#define JSON_SERIALIZE(STRUCT)                                                                     \
    inline void to_json(nlohmann::json &j, const STRUCT &t) { hana_to_json(j, t); }                \
    inline void from_json(const nlohmann::json &j, STRUCT &t) { hana_from_json(j, t); }

struct battlestats {
    BOOST_HANA_DEFINE_STRUCT(battlestats, (int, hp), (int, mp), (int, attack_min),
                             (int, attack_max), (int, inner_power), (int, accuracy), (int, evasion),
                             (int, speed), (int, defence), (int, crit_chance), (int, crit_damage),
                             (int, reduce_def), (int, reduce_def_perc), (int, hp_regen),
                             (int, mp_regen), (int, gold_res), (int, wood_res), (int, water_res),
                             (int, fire_res), (int, earth_res), (int, hp_on_hit), (int, hp_steal),
                             (int, mp_on_hit), (int, mp_steal));

    battlestats &operator+=(const battlestats &rhs) {
        boost::hana::for_each(boost::hana::keys(*this), [&](auto key) {
            auto &member = boost::hana::at_key(*this, key);
            auto &rhs_member = boost::hana::at_key(rhs, key);
            member += rhs_member;
        });
        return *this;
    }
};
JSON_SERIALIZE(battlestats);

struct attributes {
    BOOST_HANA_DEFINE_STRUCT(attributes, (int, strength), (int, dexterity), (int, physique),
                             (int, spirit));
};
JSON_SERIALIZE(attributes);

struct character {
    BOOST_HANA_DEFINE_STRUCT(character, (std::string, name), (int, character_id),
                             (attributes, attrs), (battlestats, stats), (equipment_ids, equipments),
                             (magic_ids, active_magic));
};
JSON_SERIALIZE(character);

struct magic {
    BOOST_HANA_DEFINE_STRUCT(magic, (std::string, name), (int, magic_id), (int, active), (int, cd),
                             (int, multiplier), (int, inner_damage), (int, mp_cost),
                             (property, inner_property), (battlestats, stats));
};
JSON_SERIALIZE(magic);

struct equipment {
    BOOST_HANA_DEFINE_STRUCT(equipment, (int, equipment_id), (std::string, name),
                             (std::string, description), (equipmenttype, type),
                             (battlestats, stats), (int, item_level), (int, required_level));
};
JSON_SERIALIZE(equipment);

} // namespace nibashared