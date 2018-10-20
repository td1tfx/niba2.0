#pragma once

#include "client_processor.h"
#include "global_defs.h"
#include "message.h"

#include <thread>

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

    void handle_cmd(const std::string &input) {
        std::vector<std::string> results;
        boost::split(results, input, boost::is_any_of("\t "));
        // password input handled by cmd_processor
        if (results.size() == 3) {
            if (results[0] == "register") {
                return create_and_go<nibashared::register_request>(results[1], results[2]);
            } else if (results[0] == "login") {
                return create_and_go<nibashared::login_request>(results[1], results[2]);
            }
        }
        std::cout << "incorrect command" << std::endl;
    }

private:
    template<typename Message, typename... Args>
    void create_and_go(Args &&... args) {
        Message message(std::forward<Args>(args)...);
        if (!message.validate(processor_.get_session())) {
            // TODO maybe message specific error msg
            std::cout << "command validation failed" << std::endl;
        } else {
            std::string request_str = message.create_request().dump();
            ws_.write(boost::asio::buffer(request_str));
            std::string response_str;
            auto buffer = boost::asio::dynamic_buffer(response_str);
            ws_.read(buffer);
            processor_.dispatch(message, response_str);
        }
    }

    void ping_timer(boost::system::error_code ec);

    std::string const &host_;
    std::string const &port_;
    boost::asio::io_context &ioc_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ws_;
    boost::asio::steady_timer timer_;
    // boost::asio::io_service::work work_;
    std::thread worker_;
    client_processor processor_;
};

} // namespace nibaclient