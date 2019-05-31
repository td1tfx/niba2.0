#pragma once

#include "client_processor.h"
#include "global_defs.h"
#include "message.h"
#include "structs.h"
#include "util.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <future>
#include <memory>
#include <optional>

namespace nibaclient {

class client_session : public std::enable_shared_from_this<client_session> {
public:
    client_session(std::string const &host, std::string const &port, boost::asio::io_context &ioc,
                   boost::asio::ssl::context &ssl_ctx);
    void start(); // unfortunately we need this as shared_from_this() wouldn't work in constructor
    void block_until_ready();
    void stop();
    std::future<nibashared::message::type> handle_cmd(const std::string &input);
    std::chrono::high_resolution_clock::time_point earliest() const;

private:
    template<typename Message, typename... Args, typename = nibashared::IsMessage<Message>>
    std::future<nibashared::message::type> create_and_go(Args &&... args) {
        // User will not overwrite a non-fulfilled promise as they'd have
        request_promise_ = std::promise<nibashared::message::type>();
        auto future = request_promise_.get_future();
        Message message(std::forward<Args>(args)...);
        // Post to ioc thread(or spawn coroutine) to avoid accessing data on 2 threads
        // particularly, request_ should be shielded
        boost::asio::spawn(ioc_, [this, self = shared_from_this(),
                                  message{std::move(message)}](boost::asio::yield_context yield) mutable {
            if (!message.base_validate(processor_.get_session())) {
                // TODO maybe message specific error msg
                std::cout << "command validation failed" << std::endl;
                request_promise_.set_value(nibashared::message::type::none);
            } else {
                write_message(std::move(message), yield);
            }
        });
        return future;
    }

    template<typename Message>
    void write_message(Message&& message, boost::asio::yield_context yield) {
        auto request_json = message.base_create_request();
        std::string request_str = request_json.dump();
        request_.emplace(std::move(message));
        request_stopwatch_.reset();
        boost::system::error_code ec;
        ws_.async_write(boost::asio::buffer(request_str), yield[ec]);
        if (ec) {
            // TODO figure out how to handle error properly
            std::cout << "failed to write data " << ec.message() << std::endl;
            request_promise_.set_value(nibashared::message::type::none);
            request_.reset();
        }
    }

    std::string const &host_;
    std::string const &port_;
    boost::asio::io_context &ioc_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;
    std::promise<void> ready_promise_;
    std::future<void> ready_future_;

    client_processor processor_;

    std::optional<nibashared::message::variant> request_;
    std::promise<nibashared::message::type> request_promise_;
    nibautil::stopwatch request_stopwatch_;

    bool handled_ = false;
};

} // namespace nibaclient
