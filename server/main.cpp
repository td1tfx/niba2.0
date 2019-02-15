//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket SSL server, coroutine
//
//------------------------------------------------------------------------------

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <ozo/connection.h>
#include <ozo/connection_info.h>
#include <ozo/connection_pool.h>
#include <ozo/execute.h>
#include <ozo/request.h>
#include <ozo/shortcuts.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
// #include <thread>
// #include <vector>

#include "cert_loader.hpp"
#include "db_accessor.h"
#include "gamedata.h"
#include "logger.h"
#include "server_session.h"
#include "structs.h"

using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;              // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>
namespace sev = boost::log::trivial;
using namespace ozo::literals;
using namespace nibaserver;

// TODO maybe refactor it a bit
void init_gamedata() {
    using namespace nibashared;
    // read from postgres here
    const auto connection_info = ozo::make_connection_info("dbname=niba_static user=postgres");
    ozo::io_context ioc;
    const auto connector = ozo::make_connector(connection_info, ioc);
    ozo::rows_of<std::string> character, magic, item;

    boost::asio::spawn(ioc, [&](boost::asio::yield_context yield) {
        ozo::request(connector, "SELECT to_json::TEXT FROM character_dump"_SQL,
                     ozo::into(character), yield);

        ozo::request(connector, "SELECT to_json::TEXT FROM magic_dump"_SQL, ozo::into(magic),
                     yield);

        ozo::request(connector, "SELECT to_json::TEXT FROM item_dump"_SQL, ozo::into(item), yield);
    });
    ioc.run();

    staticdata::init([&character, &magic, &item](auto &characters, auto &magics, auto &equipments) {
        try {
            nlohmann::json serialized_chars = nlohmann::json::parse(std::get<0>(character.at(0)));
            for (auto &element : serialized_chars) {
                characters[element["character_id"]] = element;
            }

            nlohmann::json serialized_magic = nlohmann::json::parse(std::get<0>(magic.at(0)));
            for (auto &element : serialized_magic) {
                magics[element["magic_id"]] = element;
            }

            nlohmann::json serialized_equipment = nlohmann::json::parse(std::get<0>(item.at(0)));
            for (auto &element : serialized_equipment) {
                equipments[element["equipment_id"]] = element;
            }
        } catch (std::exception &e) {
            // I don't care at this point, this should be tested statically
            std::cout << "failed " << e.what() << std::endl;
            exit(-1);
        }
    });
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    auto const address = boost::asio::ip::make_address("0.0.0.0");
    constexpr unsigned short port = 19999;
    // Single threaded
    auto const threads = std::max<int>(1, 1);

    init_log();
    logger logger;

    init_gamedata();

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23};

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);

    BOOST_LOG_SEV(logger, sev::info) << "Server starts.";

    // Spawn a listening port, note as per clang, &port is not required in capture list as its
    // a constexpr

    auto connection_info = ozo::make_connection_info("dbname=niba user=postgres");

    ozo::connection_pool_config connection_pool_config;

    // Maximum limit for number of stored connections
    connection_pool_config.capacity = 100;
    // Maximum limit for number of waiting requests for connection
    connection_pool_config.queue_capacity = 5000;
    // Maximum time duration to store unused open connection
    connection_pool_config.idle_timeout = std::chrono::seconds(60);

    // Creating connection pool from connection_info as the underlying ConnectionSource
    auto connection_pool = ozo::make_connection_pool(connection_info, connection_pool_config);
    ozo::connection_pool_timeouts timeouts;
    timeouts.connect = std::chrono::seconds(10);
    timeouts.queue = std::chrono::seconds(10);
    const auto connector = ozo::make_connector(connection_pool, ioc, timeouts);

    boost::asio::spawn(ioc, [&ioc, &address, &ctx, &connector,
                             &logger](boost::asio::yield_context yield) {
        boost::system::error_code ec;

        // TODO: move elsewhere
        // reset the login status on start up
        auto conn = ozo::execute(connector, "UPDATE user_id SET logged_in = false WHERE 1 = 1"_SQL,
                                 yield[ec]);
        if (ec) {
            BOOST_LOG_SEV(logger, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                              << " | " << ozo::get_error_context(conn);
            return;
        }

        // Open the acceptor
        tcp::acceptor acceptor(ioc);
        tcp::endpoint endpoint{address, port};
        acceptor.open(endpoint.protocol());

        // Allow address reuse
        acceptor.set_option(boost::asio::socket_base::reuse_address(true));
        // Bind to the server address
        acceptor.bind(endpoint, ec);
        if (ec) {
            BOOST_LOG_SEV(logger, sev::error) << "Binding failure: " << ec.message();
            return;
        }

        // Start listening for connections
        acceptor.listen(boost::asio::socket_base::max_listen_connections);

        for (;;) {
            tcp::socket socket(ioc);
            acceptor.async_accept(socket, yield);
            tcp::no_delay option(true);
            socket.set_option(option);
            BOOST_LOG_SEV(logger, sev::info) << "Got connection";
            nibaserver::db_accessor db(connector);
            auto session = std::make_shared<server_session>(acceptor.get_executor().context(),
                                                            std::move(socket), ctx, std::move(db));
            session->go();
        }
    });

    ioc.run();

    return EXIT_SUCCESS;
}
