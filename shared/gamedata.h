#pragma once
#include "global_defs.h"
#include "structs.h"
#include <boost/assert.hpp>
#include <unordered_map>
#include <utility>

// this is temporary

namespace nibashared {

// meh singletons

class staticdata {
public:
    // so why can't operators be static, especially functors?
    static staticdata &get();
    std::tuple<std::unordered_map<int, nibashared::character>,
               std::unordered_map<int, nibashared::magic>,
               std::unordered_map<int, nibashared::equipment>>
    all();
    const nibashared::character &character(int id);
    const nibashared::magic &magic(int id);
    const nibashared::equipment &equipment(int id);
    // TODO more elegant way to do this
    // just give const access to the members?
    bool has_character(int id);
    bool has_magic(int id);
    bool has_equipment(int id);

    // MUST init somewhere before use!
    template<typename Init>
    static void init(Init init) {
        auto &inst = staticdata::get();
        init(inst.characters_, inst.magics_, inst.equipments_);
    }

private:
    staticdata();
    std::unordered_map<int, nibashared::character> characters_;
    std::unordered_map<int, nibashared::magic> magics_;
    std::unordered_map<int, nibashared::equipment> equipments_;
};

} // namespace nibashared
