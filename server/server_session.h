#pragma once

#include "global_defs.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/stream.hpp>

namespace nibaserver {

class server_session : public std::enable_shared_from_this<server_session>
{
public:
    server_session(boost::asio::io_context& ioc,
        boost::asio::ip::tcp::socket&& socket,
        boost::asio::ssl::context& ctx);
    ~server_session() = default;
    void go();
private:
    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::socket socket_;
    boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>> ws_;
};

}
