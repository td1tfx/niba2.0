#pragma once

#include "global_defs.h"
#include <chrono>

namespace nibashared {
namespace delay {
using namespace std::chrono_literals;
// all in ms
constexpr auto per_hp = std::chrono::milliseconds(static_cast<int>(100 * DELAYFACTOR));
constexpr auto fight_tick = std::chrono::milliseconds(static_cast<int>(10 * DELAYFACTOR));
} // namespace delay

// return duration, hp, ticks etc...
inline auto fight_delay(int max_hp, int hp, int ticks) {
    if (hp < 0) {
        hp = 0;
    }
    auto delay = ticks * delay::fight_tick;
    delay += delay::per_hp * static_cast<int>(((max_hp - hp) / max_hp) * 100);
    return delay;
}

} // namespace nibashared