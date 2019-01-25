#pragma once

#include <iostream>
#include <optional>
#include <string>

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

} // namespace nibashared
