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

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "cert_loader.hpp"
#include "logger.h"
#include "server_session.h"

using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;              // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>
namespace sev = boost::log::trivial;
using namespace nibaserver;

int main(int argc, char *argv[]) {
    auto const address = boost::asio::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(19999);
    // Single threaded
    auto const threads = std::max<int>(1, 1);

    init_log();
    logger logger;

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23};

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);

    BOOST_LOG_SEV(logger, sev::info) << "Server starts.";
    // Spawn a listening port
    boost::asio::spawn(
        ioc, [&ioc, &address, &port, &ctx, &logger](boost::asio::yield_context yield) {
            boost::system::error_code ec;

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
                BOOST_LOG_SEV(logger, sev::info) << "Got connection";
                auto session = std::make_shared<server_session>(acceptor.get_executor().context(),
                                                                std::move(socket), ctx);
                session->go();
            }
        });

    ioc.run();

    return EXIT_SUCCESS;
}
