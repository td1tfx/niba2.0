#pragma once

#include <ozo/connection.h>
#include <ozo/connection_info.h>
#include <ozo/connection_pool.h>
#include <structs.h>

OZO_PG_DEFINE_CUSTOM_TYPE(nibashared::attributes, "character_four_attributes")
OZO_PG_DEFINE_CUSTOM_TYPE(nibashared::player, "character_info")
OZO_PG_DEFINE_CUSTOM_TYPE(nibashared::battlestats, "battlestats")
OZO_PG_DEFINE_CUSTOM_TYPE(nibashared::magic, "magic_info")

namespace nibaserver {

// we use this just to get around needing to write out the type for the connector
inline static auto make_ozo_connector(boost::asio::io_context &ioc,
                                      const std::string &player_conn_str) {
    // note these are static so they still exist after we return from this function
    static auto connection_info = ozo::make_connection_info(
        player_conn_str, ozo::register_types<nibashared::attributes, nibashared::magic,
                                             nibashared::player, nibashared::battlestats>());
    static ozo::connection_pool_config connection_pool_config;
    static auto connection_pool =
        ozo::make_connection_pool(connection_info, connection_pool_config);
    // Creating connection pool from connection_info as the underlying ConnectionSource
    static ozo::connection_pool_timeouts timeouts;

    // Maximum limit for number of stored connections
    connection_pool_config.capacity = 100;
    // Maximum limit for number of waiting requests for connection
    connection_pool_config.queue_capacity = 5000;
    // Maximum time duration to store unused open connection
    connection_pool_config.idle_timeout = std::chrono::seconds(60);

    timeouts.connect = std::chrono::seconds(10);
    timeouts.queue = std::chrono::seconds(10);

    static auto connector = ozo::make_connector(connection_pool, ioc, timeouts);
    return connector;
}

using niba_ozo_connector = std::invoke_result_t<decltype(&make_ozo_connector),
                                                boost::asio::io_context &, const std::string &>;

} // namespace nibaserver