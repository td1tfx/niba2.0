#pragma once

#include "db_accessor.h"
#include "global_defs.h"
#include "logger.h"
#include "session_map.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <queue>
#include <string>

namespace nibaserver {

class server_session : public std::enable_shared_from_this<server_session> {
public:
    server_session(boost::asio::io_context &ioc, boost::asio::ip::tcp::socket &&socket,
                   boost::asio::ssl::context &ctx, nibaserver::db_accessor &&db,
                   session_map &ss_map);
    ~server_session();
    void go();

    // Note this call is thread safe
    // The queue is protected by the strand_
    void write(std::string str);

private:
    boost::asio::io_context &ioc_;
    boost::asio::io_context::strand strand_;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;
    nibaserver::db_accessor db_;
    session_map &ss_map_;
    logger logger_;
    std::queue<std::string> write_queue_;
};

} // namespace nibaserver
