#pragma once

#include <iostream>
#include <optional>
#include <string>

#ifdef NIBACLIENT
#define CPRINT(x) std::cout << x << std::endl;
#else
#define CPRINT(X)
#endif

#ifndef DELAYFACTOR
#define DELAYFACTOR 1.0
#endif

namespace nibashared {

enum class gamestate : std::size_t {
    prelogin = 0,
    createchar = 1,
    ingame = 2,

    LAST
};

namespace delay {
// all in ms
constexpr double per_hp = 100.0 * DELAYFACTOR;
constexpr double fight_tick = 10.0 * DELAYFACTOR;
} // namespace delay

} // namespace nibashared
