#include "server_session.h"
#include "server_processor.h"
#include "db_accessor.h"
#include "request_dispatcher.h"

#include <boost/asio/spawn.hpp>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>
using namespace nibaserver;

server_session::server_session(boost::asio::io_context & ioc, 
    boost::asio::ip::tcp::socket && socket, boost::asio::ssl::context & ctx)
    : ioc_(ioc), socket_(std::move(socket)), ws_(socket_, ctx)
{
}

void server_session::go()
{
    auto self(shared_from_this());
    boost::asio::spawn(ioc_, [this, self](boost::asio::yield_context yield) {
        nibaserver::server_processor processor;
        try {
            ws_.next_layer().async_handshake(ssl::stream_base::server, yield);
            // Accept the websocket handshake
            ws_.async_accept(yield);
            nibashared::request_dispatcher dispatcher(processor);
            for (;;)
            {
                std::string request_str;
                auto buffer = boost::asio::dynamic_buffer(request_str);
                ws_.async_read(buffer, yield);
                std::string result = dispatcher.dispatch(request_str);
                auto result_buffer = boost::asio::buffer(result);
                ws_.async_write(result_buffer, yield);
            }
        }
        // unrecoverable error
        catch (std::exception& e) {
            if (processor.get_session().id.has_value()) {
                nibaserver::db_accessor::logout(processor.get_session().id.value());
            }
            std::cerr << "session ended " << e.what() << std::endl;
        }
    });
}