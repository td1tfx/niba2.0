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
// Example: WebSocket SSL client, coroutine
//
//------------------------------------------------------------------------------

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <iostream>
#include <string>
#include <thread>

#include "cert_loader.hpp"
#include "client_session.h"

using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;              // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>

//------------------------------------------------------------------------------

int main(int argc, char **argv) {
    std::string host = "localhost";
    std::string port = "19999";

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23_client};

    // ctx.load_verify_file("server.crt");
    load_root_certificates(ctx);
    ctx.set_verify_mode(ssl::verify_peer);

    boost::asio::io_context ioc;
    nibaclient::client_session client_session(host, port, ioc, ctx);

    std::thread([&ioc, &client_session] {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == "exit" || line == "quit")
                break;
            try {
                // post the work to ioc
                // we do not do any network io here
                ioc.post([&client_session, line]() { client_session.handle_cmd(line); });
            } catch (std::exception &e) {
                std::cout << e.what() << std::endl;
                break;
            }
        }
        ioc.stop();
    })
        .detach();

    ioc.run();

    return EXIT_SUCCESS;
}
