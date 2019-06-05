#pragma once

#include "connector.h"
#include "logger.h"
#include "structs.h"
#include <boost/asio/spawn.hpp>
#include <optional>
#include <ozo/connection.h>
#include <ozo/connection_info.h>
#include <ozo/connection_pool.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace nibaserver {

class db_accessor {
public:
    db_accessor(niba_ozo_connection_pool_ref &conn_pool, boost::asio::io_context &ioc);
    // ~db_accessor() = default;
    bool login(const std::string &id, const std::string &password,
               boost::asio::yield_context &yield);

    bool logout(const std::string &id, boost::asio::yield_context &yield);

    bool create_user(const std::string &id, const std::string &password,
                     boost::asio::yield_context &yield);

    // NOTE no save char functions yet
    std::optional<nibashared::player> get_char(const std::string &id,
                                               boost::asio::yield_context &yield);
    // get player auxiliary data, magic and equipments etc.
    nibashared::playerdata get_aux(const std::string &name, boost::asio::yield_context &yield);
    // create new character
    bool create_char(const std::string &id, const nibashared::player &player,
                     boost::asio::yield_context &yield);

    // Do in memory check before calling create_magic!
    // Note the created magic will not be equipped by default
    bool create_magic(const std::string &player_name, const nibashared::magic &magic,
                      boost::asio::yield_context &yield);
    // To update a magic with new data
    // Its possible that the deleting magic is from a equipped magic
    // So we also obtain the new list
    bool fuse_magic(const std::string &player_name, const nibashared::magic &magic,
                    int delete_magic_id, const std::vector<int> &equipped_magic_ids,
                    boost::asio::yield_context &yield);
    bool equip_magics(const std::string &player_name, const std::vector<int> &equipped_magic_ids,
                      boost::asio::yield_context &yield);

private:
    auto get_connector() {
        return conn_pool_[ioc_];
    }

    logger logger_;
    niba_ozo_connection_pool_ref& conn_pool_;
    boost::asio::io_context& ioc_;
};

} // namespace nibaserver