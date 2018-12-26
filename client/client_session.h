#pragma once

#include "client_processor.h"
#include "global_defs.h"
#include "message.h"
#include "structs.h"

#include <chrono>
#include <ratio>
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
        if (input == "exit") {
            std::cout << "goodbye" << std::endl;
            // this might not be the last command... but works for tests
            close();
            ioc_.stop();
            return;
        }
        try {
            std::vector<std::string> results;
            boost::split(results, input, boost::is_any_of("\t "));
            // password input handled by cmd_processor
            if (results.size() == 3) {
                if (results[0] == "register") {
                    return create_and_go<nibashared::message_register>(std::move(results[1]),
                                                                       std::move(results[2]));
                } else if (results[0] == "login") {
                    return create_and_go<nibashared::message_login>(std::move(results[1]),
                                                                    std::move(results[2]));
                }
            } else if (results.size() == 2) {
                if (results[0] == "fight") {
                    return create_and_go<nibashared::message_fight>(std::stoi(results[1]));
                }
            } else if (results.size() == 7) {
                if (results[0] == "create") {
                    return create_and_go<nibashared::message_createchar>(
                        std::move(results[1]), std::stoi(results[2]),
                        nibashared::attributes{.strength = std::stoi(results[3]),
                                               .dexterity = std::stoi(results[4]),
                                               .physique = std::stoi(results[5]),
                                               .spirit = std::stoi(results[6])});
                }
            }
        } catch (...) {
            // parsing failure whatever
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
            auto t1 = std::chrono::high_resolution_clock::now();
            ws_.write(boost::asio::buffer(request_str));
            std::string response_str;
            auto buffer = boost::asio::dynamic_buffer(response_str);
            ws_.read(buffer);
            auto t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
            std::cerr << fp_ms.count() << std::endl;
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
};

} // namespace nibaclient
