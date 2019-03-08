#pragma once
#include "global_defs.h"
#include "structs.h"
#include <chrono>
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
    std::vector<int> equipped_magic_ids;

    std::chrono::high_resolution_clock::time_point current_time; // when request comes in
    std::chrono::high_resolution_clock::time_point
        earliest_time; // add delay to current_time if neccessary
};

struct playerdata {
    // db_accessor can just return playerdata
    std::optional<std::string> userid;
    std::optional<nibashared::player> player;
    std::vector<magic> magics;
    std::vector<equipment> equips;
    std::vector<int> equipped_magic_ids;
};

} // namespace nibashared