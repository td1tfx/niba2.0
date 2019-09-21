// this file is generated, see structs.yaml and gen_structs.py

#pragma once

#include <array>
#include <boost/hana.hpp>
#include <cmath>
#include <nlohmann/json.hpp>
#include <ostream>

// Copied from nlohmann readme
namespace nlohmann {
template<typename T>
struct adl_serializer<std::optional<T>> {
    static void to_json(json &j, const std::optional<T> &opt) {
        if (!opt) {
            j = nullptr;
        } else {
            j = *opt; // this will call adl_serializer<T>::to_json which will
                      // find the free function to_json in T's namespace!
        }
    }

    static void from_json(const json &j, std::optional<T> &opt) {
        if (j.is_null()) {
            opt = {};
        } else {
            opt = j.get<T>(); // same as above, but with
                              // adl_serializer<T>::from_json
        }
    }
};
} // namespace nlohmann

namespace nibashared {

using magic_ids = std::vector<int>;
using equipment_ids = std::vector<int>;
using enemy_ids = std::vector<int>;
using map_ids = std::vector<int>;

enum class property { gold = 'j', wood = 'm', water = 's', fire = 'h', earth = 't' };
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
constexpr auto HanaStruct = boost::hana::Struct<std::decay_t<T>>::value;

template<typename T>
using IsHanaStructT = std::enable_if_t<HanaStruct<T>, T>;

template<typename T, typename = IsHanaStructT<T>>
void to_json(nlohmann::json &j, const T &t) {
    boost::hana::for_each(t, boost::hana::fuse([&j](auto name, auto member) {
                              j[boost::hana::to<char const *>(name)] = member;
                          }));
}

template<typename T, typename = IsHanaStructT<T>>
void from_json(const nlohmann::json &j, T &t) {
    boost::hana::for_each(boost::hana::keys(t), [&j, &t](auto key) {
        auto &member = boost::hana::at_key(t, key);
        j.at(boost::hana::to<char const *>(key)).get_to(member);
    });
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
    os << "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        os << v[i];
        if (i != v.size() - 1)
            os << ", ";
    }
    os << "] ";
    return os;
}

template<typename T, typename = IsHanaStructT<T>>
std::ostream &operator<<(std::ostream &os, const T &t) {
    boost::hana::for_each(t, boost::hana::fuse([&os](auto name, auto member) {
                              os << boost::hana::to<char const *>(name) << ": " << member << " ";
                          }));
    return os;
}

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

    battlestats &operator*=(double multiplier) {
        boost::hana::for_each(boost::hana::keys(*this), [&](auto key) {
            auto &member = boost::hana::at_key(*this, key);
            member = std::lround(member * multiplier);
        });
        return *this;
    }
};

struct attributes {
    BOOST_HANA_DEFINE_STRUCT(attributes, (int, strength), (int, dexterity), (int, physique),
                             (int, spirit));
    // Better to use iterator
    // Or use ranges and coroutines once they come out
    using array_ref = std::array<std::reference_wrapper<int>, 4>;
    array_ref get_array_ref() {
        return {std::ref(strength), std::ref(dexterity), std::ref(physique), std::ref(spirit)};
    }
    enum class selector : std::size_t { strength = 0, dexterity = 1, physique = 2, spirit = 3 };
};

struct character {
    BOOST_HANA_DEFINE_STRUCT(character, (std::string, name), (int, character_id),
                             (std::string, description), (attributes, attrs), (battlestats, stats),
                             (equipment_ids, equipments), (magic_ids, active_magic));
};

// adapters? or just use character?
struct player {
    // may contain other stuff, but whatever
    BOOST_HANA_DEFINE_STRUCT(player, (std::string, name), (char, gender), (attributes, attrs));
};

struct magic {
    BOOST_HANA_DEFINE_STRUCT(magic, (int, magic_id), (std::string, name), (int, active),
                             (int, multiplier), (int, inner_damage), (int, cd), (int, mp_cost),
                             (char, inner_property), (std::string, description),
                             (battlestats, stats));
};

struct equipment {
    BOOST_HANA_DEFINE_STRUCT(equipment, (int, equipment_id), (int, static_id), (std::string, name),
                             (std::string, description), (equipmenttype, type),
                             (battlestats, stats), (int, item_level), (int, required_level));
};

struct map {
    BOOST_HANA_DEFINE_STRUCT(map, (int, map_id), (std::string, name), (std::string, description),
                             (double, elite_prob), (double, boss_prob), (int, boss_id),
                             (enemy_ids, enemies), (map_ids, open_maps), (int, is_open));
};

struct playerdata {
    BOOST_HANA_DEFINE_STRUCT(playerdata, (std::vector<magic>, magics),
                             (std::vector<equipment>, equips), (magic_ids, equipped_magic_ids),
                             (map_ids, avail_map_ids));
};

} // namespace nibashared
