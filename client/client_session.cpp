#include "client_session.h"

using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;              // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>

using namespace nibaclient;

// , work_(ioc_)
nibaclient::client_session::client_session(std::string const &host, std::string const &port,
                                           boost::asio::io_context &ioc,
                                           boost::asio::ssl::context &ssl_ctx) :
    host_(host),
    port_(port), ioc_(ioc), resolver_(ioc_), ws_(ioc_, ssl_ctx), timer_(ioc_) {
    // everything can be sync because this client has no ui anyway
    auto const results = resolver_.resolve(host_, port_);
    boost::asio::connect(ws_.next_layer().next_layer(), results.begin(), results.end());
    ws_.next_layer().handshake(ssl::stream_base::client);
    ws_.handshake(host_, "/");
    timer_.expires_after(std::chrono::seconds(5));
    ping_timer({});
}

nibaclient::client_session::~client_session() { close(); }

void nibaclient::client_session::close() {
    if (ws_.is_open()) {
        ws_.close(websocket::close_code::normal);
    }
    timer_.cancel();
}

void nibaclient::client_session::ping_timer(boost::system::error_code ec) {
    if (ec && ec != boost::asio::error::operation_aborted)
        return;

    // See if the timer really expired since the deadline may have moved.
    if (timer_.expiry() <= std::chrono::steady_clock::now()) {
        // If this is the first time the timer expired,

        if (ws_.is_open()) {
            timer_.expires_after(std::chrono::seconds(5));
            // Now send the ping
            ws_.async_pong({}, [](boost::system::error_code ec) {
                (void)ec;
                // std::cout << "ping sent" << std::endl;
            });
        }
    }

    // Wait on the timer
    timer_.async_wait([this](boost::system::error_code ec) { ping_timer(ec); });
}
