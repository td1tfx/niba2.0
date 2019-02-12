#pragma once
#include "global_defs.h"
#include "structs.h"
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace nibashared {

struct sessionstate {
    // this constructor might have problems, whatever
    sessionstate() : state(nibashared::gamestate::prelogin) {}
    gamestate state;
    std::optional<std::string> userid;
    std::optional<nibashared::player> player;
    std::vector<magic> magics;
    std::vector<equipment> equips;
};

} // namespace nibashared