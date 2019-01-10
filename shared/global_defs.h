#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#ifdef NIBACLIENT
#define CPRINT(x) std::cout << x << std::endl;
#else
#define CPRINT(X)
#endif

namespace nibashared {

enum class gamestate : std::size_t {
    prelogin = 0,
    createchar = 1,
    ingame = 2,

    LAST
};

struct sessionstate {
    gamestate state;
    std::optional<std::string> userid;
    int charid{-1};
};

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

} // namespace nibashared
