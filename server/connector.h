#pragma once

#include <ozo/connection.h>
#include <ozo/connection_info.h>
#include <ozo/connection_pool.h>
#include <ozo/execute.h>
#include <ozo/request.h>
#include <ozo/shortcuts.h>
#include <type_traits>

namespace nibaserver {

// we use this just to get around needing to write out the type for the connector
inline auto make_ozo_connector(boost::asio::io_context &ioc) {
    // note these are static so they still exist after we return from this function
    static auto connection_info = ozo::make_connection_info("dbname=niba user=postgres");
    ozo::connection_pool_config connection_pool_config;
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

    return ozo::make_connector(connection_pool, ioc, timeouts);
}

using niba_ozo_connector =
    std::invoke_result_t<decltype(&make_ozo_connector), boost::asio::io_context&>;

} // namespace nibaserver