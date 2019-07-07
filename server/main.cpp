#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <ozo/execute.h>
#include <ozo/request.h>
#include <ozo/shortcuts.h>

#include <cstdlib>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "cert_loader.hpp"
#include "config.h"
#include "connector.h"
#include "data_init.h"
#include "db_accessor.h"
#include "gamedata.h"
#include "logger.h"
#include "server_session.h"
#include "session_map.h"

using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;              // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>
namespace sev = boost::log::trivial;
using namespace nibaserver;

int main(int argc, char *argv[]) {
    config conf = read_config(argc, argv);

    auto const address = boost::asio::ip::make_address(conf.host);
    unsigned short port = conf.port;
    auto const threads = conf.threads;

    init_log();
    logger logger;
    BOOST_LOG_SEV(logger, sev::info) << "Server starts.";

    init_gamedata(conf.static_conn_str);
    BOOST_LOG_SEV(logger, sev::info) << "Game data loaded.";

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23};

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);
    auto connection_pool = make_ozo_connector_pool(conf.player_conn_str);

    // We need a global session weakptr to handle player connections
    // and a timer to periodically cleanup the weakptr
    // The key of the mapping would be the user_id
    // the global sessions will be passed into the server_session?
    session_map ss_map;

    boost::asio::spawn(ioc, [&ioc, &address, &port, &ctx, &connection_pool,
                             &logger, &ss_map](boost::asio::yield_context yield) {
        using namespace ozo::literals;
        boost::system::error_code ec;

        // TODO: move elsewhere
        // reset the login status on start up
        auto conn = ozo::execute(connection_pool[ioc], "UPDATE user_id SET logged_in = false WHERE 1 = 1"_SQL,
                                 ozo::deadline(std::chrono::seconds(5)),
                                 yield[ec]);
        if (ec) {
            BOOST_LOG_SEV(logger, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                              << " | " << ozo::get_error_context(conn);
            return;
        }
        BOOST_LOG_SEV(logger, sev::info) << "All users logged out.";

        // Open the acceptor
        tcp::acceptor acceptor{ioc};
        tcp::endpoint endpoint{address, port};
        acceptor.open(endpoint.protocol());

        // Allow address reuse
        acceptor.set_option(boost::asio::socket_base::reuse_address(true));
        // Bind to the server address
        acceptor.bind(endpoint);

        // Start listening for connections
        acceptor.listen(boost::asio::socket_base::max_listen_connections);

        BOOST_LOG_SEV(logger, sev::info) << "Listening for connection on " << port;

        for (;;) {
            tcp::socket socket{ioc};
            acceptor.async_accept(socket, yield);
            tcp::no_delay option(true);
            socket.set_option(option);
            BOOST_LOG_SEV(logger, sev::info) << "Got connection";
            nibaserver::db_accessor db{connection_pool, ioc};
            auto session = std::make_shared<server_session>(ioc, std::move(socket), ctx,
                                                            std::move(db), ss_map);
            session->go();
        }
    });

    // session_ptr clean up
    constexpr auto interval = std::chrono::minutes(5);  // TODO: make it configurable
    boost::asio::steady_timer cleanup_timer{ioc};
    cleanup_timer.expires_after(interval);
    boost::asio::spawn(ioc, [&cleanup_timer, &ss_map](boost::asio::yield_context yield) {
        for (;;) {
            cleanup_timer.async_wait(yield);
            ss_map.cleanup();
            cleanup_timer.expires_after(interval);
        }
    });


    BOOST_LOG_SEV(logger, sev::info) << "Main thread ioc running.";
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });
    ioc.run();

    return EXIT_SUCCESS;
}
