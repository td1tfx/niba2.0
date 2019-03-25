#pragma once
#include "global_defs.h"
#include "structs.h"
#include <boost/assert.hpp>
#include <unordered_map>
#include <utility>

// this is temporary

namespace nibashared {

class staticdata {
public:
    template<typename T>
    using internal_map = std::unordered_map<int, T>;

    static staticdata &get() {
        static staticdata instance;
        return instance;
    }

    template<typename T>
    const T &at(int id) {
        return get_map_<T>().at(id);
    }

    template<typename T>
    void to(int id, T &t) {
        t = get_map_<T>().at(id);
    }

    template<typename T>
    bool has(int id) {
        return get_map_<T>().find(id) != get_map_<T>().end();
    }

    template<typename T>
    T map() {
        return get_map_<T>();
    }

    template<typename T>
    void to_map(internal_map<T> &t) {
        t = get_map_<T>();
    }

    // MUST init somewhere before use!
    template<typename... Ts>
    static void init(Ts&&... maps) {
        auto &inst = staticdata::get();
        auto result = {(inst.set_map(std::forward<Ts>(maps)), 0)...};
        (void)result;
    }

private:
    staticdata() = default;

    template<typename T>
    auto &get_map_() {
        static internal_map<T> internal;
        return internal;
    }

    template<typename T>
    void set_map(internal_map<T> &&mapper) {
        get_map_<T>() = std::move(mapper);
    }
};

constexpr auto getdata = staticdata::get;

} // namespace nibashared
