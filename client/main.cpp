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

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "cert_loader.hpp"
#include "client_session.h"
#include "global_defs.h"
#include "util.h"

using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;              // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>

//------------------------------------------------------------------------------

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    std::unique_ptr<std::ifstream> fin;
    if (argc == 2) {
        fin = std::make_unique<std::ifstream>(argv[1]);
    }
    std::istream &instream = fin ? *fin : std::cin;

    std::string host = "localhost";
    std::string port = "19999";

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23_client};

    // ctx.load_verify_file("server.crt");
    load_root_certificates(ctx);
    ctx.set_verify_mode(ssl::verify_peer);

    boost::asio::io_context ioc;
    auto work_guard = boost::asio::make_work_guard(ioc);
    nibaclient::client_session client_session(host, port, ioc, ctx);

    std::thread cmd_thread([&ioc, &work_guard, &instream, &client_session] {
        // separate io thread so that getline doesn't block our websocket pingpong
        std::string line;
        std::getline(instream, line);
        for (; instream;) {
            if (line.empty())
                continue;
            if (line == "exit" || line == "quit")
                break;
            auto earliest = client_session.earliest();
            auto now = std::chrono::high_resolution_clock::now();
            if (earliest > now) {
                // if we do auto its nano seconds, fine
                auto delay = earliest - now;
                std::cout << "cooldown " << delay.count() << "ns" << std::endl;
                std::this_thread::sleep_for(delay);
            }
            std::promise<void> promise;
            auto future = promise.get_future();
            boost::asio::post(ioc, [&client_session, line, promise{std::move(promise)}]() mutable {
                // sets the promise once its's done
                client_session.handle_cmd(line, std::move(promise));
            });
            std::getline(instream, line);
            future.wait();
        }
        work_guard.reset();
        ioc.stop();
    });

    ioc.run();
    if (cmd_thread.joinable()) {
        cmd_thread.join();
    }

    return EXIT_SUCCESS;
}
