#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace nibashared {

enum class gamestate : std::size_t {
    prelogin = 0,
    selectchar = 1,

    LAST
};

struct sessionstate {
    gamestate state;
    std::optional<std::string> userid;
    int charid{-1};
};

// from effective modern c++
template<typename E>
constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

struct battlestats {
    int attack_min{0};
    int attack_max{0};
    int accuracy{0};
    int evasion{0};
    int speed{0};
    int hp{0};
    int mp{0};
    int inner_power{0};
    int defence{0};

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
        return *this;
    }
};

constexpr std::size_t MAX_ACTIVE_MAGIC = 5;

using character_magic_ids = std::array<int, MAX_ACTIVE_MAGIC>;

// a character
struct character {
    std::string_view name;
    int id; // definitely unique

    // attributes
    int strength;
    int dexterity;
    int physique;
    int spirit;

    battlestats stats;

    // since we will be passing this around
    // it's probably a better idea to use ids
    character_magic_ids active_magic;

    // TODO equipments
};

struct magic {
    std::string_view name;
    int id;

    int active; // has CD?
    int cd;
    int multiplier; // in percentage
    int inner_damage; // certain value

    battlestats stats;
};

constexpr magic DEFAULT_MAGIC{"default", 0, 1, 0, 100, 100, {}};

} // namespace nibashared