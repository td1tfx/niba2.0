#include "client_session.h"
#include "client_processor.h"
#include "cmd_processor.h"

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

using namespace nibaclient;

nibaclient::client_session::client_session(std::string const & host, std::string const & port, boost::asio::io_context & ioc, boost::asio::ssl::context & ctx) :
    host_(host), port_(port), ioc_(ioc), resolver_(ioc_), ws_( ioc_, ctx )
{
}

void client_session::go()
{
    auto self(shared_from_this());
    boost::asio::spawn(ioc_, [this, self](boost::asio::yield_context yield) {
        try {
            // Look up the domain name
            auto const results = resolver_.async_resolve(host_, port_, yield);

            // Make the connection on the IP address we get from a lookup
            boost::asio::async_connect(ws_.next_layer().next_layer(), results.begin(), results.end(), yield);

            // Perform the SSL handshake
            ws_.next_layer().async_handshake(ssl::stream_base::client, yield);

            // Perform the websocket handshake
            ws_.async_handshake(host_, "/", yield);

            client_processor processor;

            for (;;) {
                std::string txt;
                if (!std::getline(std::cin, txt)) {
                    break;
                }
                if (txt == "exit" || txt == "quit") {
                    break;
                }

                auto communication_handler = [this, &yield, &processor](auto& message) {
                    // the client doesn't have to be async
                    std::string request_str = message.create_request().dump();
                    ws_.async_write(boost::asio::buffer(request_str), yield);
                    std::string response_str;
                    auto buffer = boost::asio::dynamic_buffer(response_str);
                    ws_.async_read(buffer, yield);
                    processor.dispatch(message, response_str);
                };

                cmd_processor::handle_cmd(txt, processor, communication_handler);

            }
        }
        catch (std::exception& e) {
            std::cout << "client failed " << e.what() << std::endl;
        }
        // Close the WebSocket connection
        ws_.async_close(websocket::close_code::normal, yield);
    });
}
