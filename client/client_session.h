#pragma once

#include "global_defs.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

namespace nibaclient {

class client_session : public std::enable_shared_from_this<client_session>
{
public:
    client_session(std::string const& host, std::string const& port,
        boost::asio::io_context& ioc, boost::asio::ssl::context& ctx);
    ~client_session() = default;
    void go();


private:
    std::string const& host_;
    std::string const& port_;
    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ws_;
};

}