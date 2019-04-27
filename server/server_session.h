#pragma once

#include "db_accessor.h"
#include "global_defs.h"
#include "logger.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
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
    boost::asio::io_context &ioc_;
    boost::asio::io_context::strand strand_;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;
    nibaserver::db_accessor db_;
    logger logger_;
};

} // namespace nibaserver
