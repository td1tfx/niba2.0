// https://github.com/boostorg/beast/blob/develop/example/websocket/client/coro-ssl/websocket_client_coro_ssl.cpp

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <chrono>
#include <condition_variable>
// #include <fstream>
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

int main(int argc, char **argv) {
    // std::unique_ptr<std::ifstream> fin{};
    // if (argc == 2) {
    //     fin = std::make_unique<std::ifstream>(argv[1]);
    // }
    // std::istream &instream = fin ? *fin : std::cin;

    // disable fstream in for now
    std::istream &instream = std::cin;

    std::string host{"localhost"};
    std::string port{"19999"};
    if (argc >= 2) {
        host = argv[1];
    }
    if (argc == 3) {
        port = argv[2];
    }

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23_client};

    // ctx.load_verify_file("server.crt");
    load_root_certificates(ctx);
    ctx.set_verify_mode(ssl::verify_peer);

    boost::asio::io_context ioc{};
    auto work_guard = boost::asio::make_work_guard(ioc);

    std::thread io_worker([&ioc] { ioc.run(); });

    auto session_ptr = std::make_shared<nibaclient::client_session>(host, port, ioc, ctx);
    // This needs to be a shared_ptr to use shared_from_this(), which makes our coroutine
    // safe to access all members at all times.
    // We can't rely on destructor for cleanup as a long running coroutine might not be done yet.
    // Another way would be that the destructor should wait for futures that the coroutine
    // would set at the end its execution.
    session_ptr->start();
    // Use start so we can fetch the first line of input while being setup

    std::string line{};

    std::getline(instream, line);     // Read line while session is starting...
    session_ptr->block_until_ready(); // Block until session is actually ready

    for (; instream;) {
        if (line.empty())
            continue;
        if (line == "exit" || line == "quit")
            break;
        auto earliest = session_ptr->earliest();
        auto now = std::chrono::high_resolution_clock::now();
        if (earliest > now) {
            // If we do auto its nano seconds, fine
            auto delay = earliest - now;
            std::cout << "cooldown " << delay.count() << "ns" << std::endl;
            std::this_thread::sleep_for(delay);
        }

        auto future = session_ptr->handle_cmd(line);

        std::getline(instream, line);

        // Blocks until request is completed, we may need better ways to handle it
        // TODO try catch, 2 types of exception: ignore, terminate
        future.wait();
    }
    // Need to stop() the session so async_read would error out and eventually stop all
    // coroutines... Also, this doesn't stop right away.
    session_ptr->stop();

    // Release guard so ioc could finish
    work_guard.reset();

    if (io_worker.joinable()) {
        io_worker.join();
    }

    return EXIT_SUCCESS;
}
