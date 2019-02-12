// this file is generated, see structs.yaml and gen_structs.py

#pragma once

#include <boost/hana.hpp>
#include <nlohmann/json.hpp>
#include <ostream>

namespace nibashared {

using magic_ids = std::vector<int>;
using equipment_ids = std::vector<int>;

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

inline std::ostream &operator<<(std::ostream &os, const property &p) {
    switch (p) {
    case property::gold:
        os << "gold";
        break;
    case property::wood:
        os << "wood";
        break;
    case property::water:
        os << "water";
        break;
    case property::fire:
        os << "fire";
        break;
    case property::earth:
        os << "earth";
        break;
    }
    return os;
}

// need a better way to do this
inline std::ostream &operator<<(std::ostream &os, const equipmenttype &v) {
    switch (v) {
    case equipmenttype::head:
        os << "head";
        break;
    case equipmenttype::armor:
        os << "armor";
        break;
    case equipmenttype::boots:
        os << "boots";
        break;
    case equipmenttype::gloves:
        os << "gloves";
        break;
    case equipmenttype::belt:
        os << "belt";
        break;
    case equipmenttype::amulet:
        os << "amulet";
        break;
    case equipmenttype::ring:
        os << "ring";
        break;
    case equipmenttype::weapon:
        os << "weapon";
        break;
    }
    return os;
}

template<typename T>
void hana_to_json(nlohmann::json &j, const T &t) {
    boost::hana::for_each(t, boost::hana::fuse([&j](auto name, auto member) {
                              j[boost::hana::to<char const *>(name)] = member;
                          }));
}

template<typename T>
void hana_from_json(const nlohmann::json &j, T &t) {
    boost::hana::for_each(boost::hana::keys(t), [&j, &t](auto key) {
        auto &member = boost::hana::at_key(t, key);
        j.at(boost::hana::to<char const *>(key)).get_to(member);
    });
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
    os << "[";
    for (std::size_t i = 0; i < v.size(); i++) {
        os << v[i];
        if (i != v.size() - 1)
            os << ", ";
    }
    os << "] ";
    return os;
}

template<typename T>
std::ostream &hana_print(std::ostream &os, const T &t) {
    // TODO constexpr or some other way to get rid of last space
    boost::hana::for_each(t, boost::hana::fuse([&os](auto name, auto member) {
                              os << boost::hana::to<char const *>(name) << ": " << member << " ";
                          }));
    return os;
}

#define STRUCT_JSON_SERIALIZE(STRUCT)                                                              \
    inline void to_json(nlohmann::json &j, const STRUCT &t) { hana_to_json(j, t); }                \
    inline void from_json(const nlohmann::json &j, STRUCT &t) { hana_from_json(j, t); }

#define STRUCT_PRINT(STRUCT)                                                                       \
    inline std::ostream &operator<<(std::ostream &os, const STRUCT &t) { return hana_print(os, t); }

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
STRUCT_JSON_SERIALIZE(battlestats);
STRUCT_PRINT(battlestats);

struct attributes {
    BOOST_HANA_DEFINE_STRUCT(attributes, (int, strength), (int, dexterity), (int, physique),
                             (int, spirit));
};
STRUCT_JSON_SERIALIZE(attributes);
STRUCT_PRINT(attributes);

struct character {
    BOOST_HANA_DEFINE_STRUCT(character, (std::string, name), (int, character_id), (std::string, description),
                             (attributes, attrs), (battlestats, stats), (equipment_ids, equipments),
                             (magic_ids, active_magic));
};
STRUCT_JSON_SERIALIZE(character);
STRUCT_PRINT(character);

// adapters? or just use character?
struct player {
    // may contain other stuff, but whatever
    BOOST_HANA_DEFINE_STRUCT(player, (std::string, name), (char, gender), (attributes, attrs));
};
STRUCT_JSON_SERIALIZE(player);
STRUCT_PRINT(player);

struct magic {
    BOOST_HANA_DEFINE_STRUCT(magic, (std::string, name), (int, magic_id), (int, static_id), (std::string, description),
                             (int, active), (int, cd), (int, multiplier), (int, inner_damage),
                             (int, mp_cost), (property, inner_property), (battlestats, stats));
};
STRUCT_JSON_SERIALIZE(magic);
STRUCT_PRINT(magic);

struct equipment {
    BOOST_HANA_DEFINE_STRUCT(equipment, (int, equipment_id), (int, static_id), (std::string, name),
                             (std::string, description), (equipmenttype, type),
                             (battlestats, stats), (int, item_level), (int, required_level));
};
STRUCT_JSON_SERIALIZE(equipment);
STRUCT_PRINT(equipment);

} // namespace nibashared