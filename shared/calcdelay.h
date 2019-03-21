#pragma once

#include "global_defs.h"
#include <chrono>

namespace nibashared {
namespace delay {
constexpr auto duration_ms(int ms) {
    return std::chrono::milliseconds(static_cast<int>(ms * DELAYFACTOR));
}

// all in ms
constexpr auto per_hp = duration_ms(100);
constexpr auto fight_tick = duration_ms(10);
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