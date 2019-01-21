#pragma once
#include "global_defs.h"
#include "structs.h"
#include <boost/assert.hpp>
#include <unordered_map>
#include <utility>

// this is temporary

namespace nibashared {

// meh singletons

class server_staticdata {
public:
    // so why can't operators be static, especially functors?
    static server_staticdata &get();
    const nibashared::character &character(int id);
    const nibashared::magic &magic(int id);
    const nibashared::equipment &equipment(int id);

private:
    server_staticdata();
    std::unordered_map<int, nibashared::character> characters_;
    std::unordered_map<int, nibashared::magic> magics_;
    std::unordered_map<int, nibashared::equipment> equipments_;
};

} // namespace nibashared
