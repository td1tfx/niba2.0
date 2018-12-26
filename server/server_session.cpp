#include "server_session.h"
#include "db_accessor.h"
#include "request_dispatcher.h"
#include "server_processor.h"

#include <boost/asio/spawn.hpp>
#include <boost/beast/http.hpp>

using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;              // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>
namespace sev = boost::log::trivial;
using namespace nibaserver;

server_session::server_session(boost::asio::io_context &ioc, boost::asio::ip::tcp::socket &&socket,
                               boost::asio::ssl::context &ctx, nibaserver::db_accessor &&db) :
    ioc_(ioc),
    socket_(std::move(socket)), ws_(socket_, ctx),
    timer_(socket_.get_executor().context(), (std::chrono::steady_clock::time_point::max)()), 
    db_(std::move(db)) {}

void server_session::go() {
    auto self(shared_from_this());
    boost::asio::spawn(ioc_, [this, self](boost::asio::yield_context yield) {
        nibaserver::server_processor processor(yield, db_);
        try {
            // start pinging
            // auto ping
            ws_.control_callback([this, self](boost::beast::websocket::frame_type kind,
                                              boost::beast::string_view payload) {
                boost::ignore_unused(kind, payload);
                ping_state_ = 0;
                timer_.expires_after(std::chrono::seconds(15));
            });
            timer_.expires_after(std::chrono::seconds(15));
            ping_timer({});
            ws_.next_layer().async_handshake(ssl::stream_base::server, yield);
            // Accept the websocket handshake
            ws_.async_accept_ex(
                [](boost::beast::websocket::response_type &m) {
                    // js-websocket(or chrome) require this field to be non-empty
                    m.insert(boost::beast::http::field::sec_websocket_protocol, "niba-server");
                },
                yield);

            nibashared::request_dispatcher dispatcher(processor);
            for (;;) {
                // recv request
                std::string request_str;
                auto buffer = boost::asio::dynamic_buffer(request_str);
                ws_.async_read(buffer, yield);

                // maintain as active
                ping_state_ = 0;
                timer_.expires_after(std::chrono::seconds(15));

                // process request and send out response
                std::string response = dispatcher.dispatch(request_str);
                ws_.async_write(boost::asio::buffer(response), yield);
            }
        }
        // unrecoverable error
        catch (std::exception &e) {
            if (processor.get_session().userid.has_value()) {
                db_.logout(yield, processor.get_session().userid.value());
            }
            BOOST_LOG_SEV(logger_, sev::info) << "Session ended, reason: " << e.what();
        }
    });
}

void server_session::ping_timer(boost::system::error_code ec) {
    if (ec && ec != boost::asio::error::operation_aborted)
        return;

    auto self(shared_from_this());

    // See if the timer really expired since the deadline may have moved.
    if (timer_.expiry() <= std::chrono::steady_clock::now()) {
        // If this is the first time the timer expired,
        // send a ping to see if the other end is there.
        if (ws_.is_open() && ping_state_ == 0) {
            // Note that we are sending a ping
            ping_state_ = 1;
            // Set the timer
            timer_.expires_after(std::chrono::seconds(15));
            // Now send the ping
            ws_.async_ping({}, [](boost::system::error_code ec) { (void)ec; });
        } else if (ws_.is_open()) {
            BOOST_LOG_SEV(logger_, sev::info) << "Connection closing";
            ws_.next_layer().next_layer().cancel();
            return;
        }
    }

    // Wait on the timer
    timer_.async_wait([this, self](boost::system::error_code ec) { ping_timer(ec); });
}
