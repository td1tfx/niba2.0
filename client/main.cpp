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
#include <iostream>
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

    std::string host = "localhost";
    std::string port = "19999";

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23_client};

    // ctx.load_verify_file("server.crt");
    load_root_certificates(ctx);
    ctx.set_verify_mode(ssl::verify_peer);

    boost::asio::io_context ioc;
    nibaclient::client_session client_session(host, port, ioc, ctx);
    std::mutex processor_mutex;
    std::condition_variable processor_cv;
    bool processed = true;

    std::thread([&ioc, &client_session, &processor_mutex, &processor_cv, &processed] {
        // separate io thread so that getline doesn't block our websocket pingpong
        std::string line;
        nibautil::stopwatch stopwatch;
        while (std::getline(std::cin, line)) {
            if (line == "exit" || line == "quit")
                break;
            std::unique_lock<std::mutex> lock(processor_mutex);
            processor_cv.wait(lock, [&processed] { return processed; });
            auto delay = client_session.get_delay();
            auto elapsed = stopwatch.elapsed_ms();
            if (delay > elapsed) { // first request should have 0 delay
                std::cout << "cooldown " << (delay - elapsed) << "ms" << std::endl;
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(static_cast<int>(delay - elapsed) + 1));
            }
            processed = false;
            try {
                // post the work to ioc
                // we do not do any network io here
                ioc.post([&client_session, &processor_mutex, &processor_cv, &processed, line]() {
                    std::unique_lock<std::mutex> lock(processor_mutex);
                    client_session.handle_cmd(line);
                    processed = true;
                    lock.unlock();
                    processor_cv.notify_one();
                });
            } catch (std::exception &e) {
                std::cout << e.what() << std::endl;
                break;
            }
            stopwatch.reset();
        }
        ioc.post([&client_session]() { client_session.handle_cmd("exit"); });
    })
        .detach();

    ioc.run();

    return EXIT_SUCCESS;
}
