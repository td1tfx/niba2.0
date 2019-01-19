#pragma once
#include "global_defs.h"
#include "structs.h"
//consider moving these to shared?
#include "../server/db_accessor.h"
#include "../server/logger.h"
#include <boost/assert.hpp>
#include <unordered_map>
#include <utility>

// this is temporary

namespace nibashared {

// meh singletons
class staticdata {
public:
    // so why can't operators be static, especially functors?
    static staticdata &get(bool is_server = false, nibaserver::db_accessor *db_ptr = NULL);
    const nibashared::character &character(int id);
    const nibashared::magic &magic(int id);
    const nibashared::equipment &equipment(int id);

private:
    staticdata(bool is_server, nibaserver::db_accessor *db_ptr);
    std::unordered_map<int, nibashared::character> characters_;
    std::unordered_map<int, nibashared::magic> magics_;
    std::unordered_map<int, nibashared::equipment> equipments_;
    void init_static_data_from_postgres(nibaserver::db_accessor db);
    void init_static_data_from_json();
};

} // namespace nibashared
