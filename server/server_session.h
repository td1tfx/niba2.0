#pragma once

#include "global_defs.h"
#include "logger.h"
#include "db_accessor.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace nibaserver {



class server_session : public std::enable_shared_from_this<server_session> {
public:
    server_session(boost::asio::io_context &ioc, boost::asio::ip::tcp::socket &&socket,
                   boost::asio::ssl::context &ctx, nibaserver::db_accessor &&db);
    ~server_session();
    void go();

private:
    enum class pingstate : char {
        responsive = 0,
        onhold = 1,
    };
    static constexpr int TIMEOUT = 15; // in seconds

    boost::asio::io_context &ioc_;
    boost::asio::io_context::strand strand_;
    boost::asio::ip::tcp::socket socket_;
    boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket &>> ws_;
    boost::asio::steady_timer timer_;
    nibaserver::db_accessor &db_;
    logger logger_;
    bool close_down_ = false;
    pingstate ping_state_ = pingstate::responsive;
};

} // namespace nibaserver
