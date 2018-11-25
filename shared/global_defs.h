#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

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

// from effective modern c++
template<typename E>
constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

} // namespace nibashared
