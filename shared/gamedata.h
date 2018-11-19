#pragma once
#include "global_defs.h"
#include "structs.h"
#include <boost/assert.hpp>
#include <vector>
#include <utility>

// this is temporary

namespace nibashared {

// meh singletons
class staticdata {
public:
    // so why can't operators be static, especially functors?
    static staticdata &get();
    const nibashared::character &character(int id);
    const nibashared::magic &magic(int id);

private:
    staticdata();
    std::vector<nibashared::character> characters_;
    std::vector<nibashared::magic> magics_;
};

} // namespace nibashared
