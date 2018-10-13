#include <boost/asio/spawn.hpp>

#include "server_session.h"

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

server_session::server_session(boost::asio::io_context & ioc, boost::asio::ip::tcp::socket && socket, boost::asio::ssl::context & ctx)
    : ioc_(ioc), socket_(std::move(socket)), ws_(socket_, ctx)
{
}

void server_session::go()
{
    auto self(shared_from_this());
    boost::asio::spawn(ioc_, [this, self](boost::asio::yield_context yield) {
        try {
            ws_.next_layer().async_handshake(ssl::stream_base::server, yield);
            // Accept the websocket handshake
            ws_.async_accept(yield);

            for (;;)
            {
                // This buffer will hold the incoming message
                boost::beast::multi_buffer buffer;

                // Read a message
                ws_.async_read(buffer, yield);

                // decode json, process

                // Echo the message back
                ws_.text(ws_.got_text());
                std::cout << boost::beast::buffers_to_string(buffer.data()) << std::endl;
                ws_.async_write(buffer.data(), yield);
            }
        }
        catch (std::exception& e) {
            std::cerr << "session ended " << e.what() << std::endl;
        }
    });
}
