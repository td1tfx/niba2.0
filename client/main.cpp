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


#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

#include "cert_loader.hpp"
#include "cmd_parser.h"
#include "cmd_processor.h"

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

//------------------------------------------------------------------------------

// Sends a WebSocket message and prints the response
void
do_session(
    std::string const& host,
    std::string const& port,
    boost::asio::io_context& ioc,
    ssl::context& ctx,
    boost::asio::yield_context yield)
{
    boost::system::error_code ec;

    // These objects perform our I/O
    tcp::resolver resolver{ ioc };
    websocket::stream<ssl::stream<tcp::socket>> ws{ ioc, ctx };

    try {
        // Look up the domain name
        auto const results = resolver.async_resolve(host, port, yield);

        // Make the connection on the IP address we get from a lookup
        boost::asio::async_connect(ws.next_layer().next_layer(), results.begin(), results.end(), yield);

        // Perform the SSL handshake
        ws.next_layer().async_handshake(ssl::stream_base::client, yield);

        // Perform the websocket handshake
        ws.async_handshake(host, "/", yield);

        for (;;) {
            std::string txt;
            if (!std::getline(std::cin, txt)) {
                break;
            }
            if (txt == "exit" || txt == "quit") {
                break;
            }

            auto request_str = nibaclient::cmd_processor::handle_cmd(txt);
            if (!request_str) {
                continue;
            }

            ws.async_write(boost::asio::buffer(*request_str), yield);

            boost::beast::multi_buffer b;
            // Read a message into our buffer
            ws.async_read(b, yield);

            // deserialize json, process it

            // The buffers() function helps print a ConstBufferSequence
            std::cout << boost::beast::buffers(b.data()) << std::endl;
        }
    }
    catch (std::exception& e) {
        std::cout << "client failed " << e.what() << std::endl;
    }
    // Close the WebSocket connection
    ws.async_close(websocket::close_code::normal, yield);
}

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    auto const host = "localhost";
    auto const port = "19999";

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ ssl::context::sslv23_client };

    // ctx.load_verify_file("server.crt");
    load_root_certificates(ctx);

    // Launch the asynchronous operation
    boost::asio::spawn(ioc, [&ioc, &ctx, &host, &port](boost::asio::yield_context yield) {
        do_session(host, port, ioc, ctx, yield);
    });

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}