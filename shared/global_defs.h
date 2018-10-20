#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace nibashared {

enum class cmdtype : std::size_t {
    login = 0,
    registeration = 1,
    create = 2,
    start = 3,

    LAST
};

enum class gamestate : std::size_t {
    prelogin = 0,
    selectchar = 1,

    LAST
};

struct sessionstate {
    gamestate state;
    std::optional<std::string> id;
};

// from effective modern c++
template<typename E>
constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

// Note, state transition may depend on the server's response
// for now, don't do anything
} // namespace nibashared