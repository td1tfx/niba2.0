#pragma once

#include "client_processor.h"
#include "global_defs.h"
#include "message.h"
#include "structs.h"
#include "util.h"

#include <boost/algorithm/string.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace nibaclient {

class client_session {
public:
    client_session(std::string const &host, std::string const &port, boost::asio::io_context &ioc,
                   boost::asio::ssl::context &ssl_ctx);
    ~client_session();

    void handle_cmd(const std::string &input);
    std::chrono::high_resolution_clock::time_point earliest() const;

private:
    template<typename Message, typename... Args>
    void create_and_go(Args &&... args) {
        Message message(std::forward<Args>(args)...);
        if (!message.base_validate(processor_.get_session())) {
            // TODO maybe message specific error msg
            std::cout << "command validation failed" << std::endl;
        } else {
            auto request_json = message.base_create_request();
            std::string request_str = request_json.dump();
            nibautil::stopwatch stopwatch;
            ws_.write(boost::asio::buffer(request_str));
            std::string response_str;
            auto buffer = boost::asio::dynamic_buffer(response_str);
            ws_.read(buffer);
            std::cerr << stopwatch.elapsed_ms() << std::endl;
            processor_.dispatch(message, response_str);
        }
    }
    void close();
    void ping_timer(boost::system::error_code ec);

    std::string const &host_;
    std::string const &port_;
    boost::asio::io_context &ioc_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ws_;
    boost::asio::steady_timer timer_;
    client_processor processor_;
    bool handled_ = false;
};

} // namespace nibaclient
