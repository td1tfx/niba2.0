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
                               boost::asio::ssl::context &ctx) :
    ioc_(ioc),
    strand_(ioc_), socket_(std::move(socket)), ws_(socket_, ctx),
    timer_(socket_.get_executor().context(), (std::chrono::steady_clock::time_point::max)()) {}

server_session::~server_session() { BOOST_LOG_SEV(logger_, sev::info) << "Session destructed"; }

void server_session::go() {
    // 2 coroutines - use same strand
    auto self1(shared_from_this());
    boost::asio::spawn(strand_, [this, self1](boost::asio::yield_context yield) {
        nibaserver::server_processor processor;
        try {
            // control callback is not a completion handler!
            ws_.control_callback([this](boost::beast::websocket::frame_type kind,
                                        boost::beast::string_view payload) {
                // control frames here are pongs
                boost::ignore_unused(kind, payload);
                ping_state_ = pingstate::responsive; // responsive
                timer_.expires_after(std::chrono::seconds(TIMEOUT));
            });
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
                // reset ping state, and timer as well
                ping_state_ = pingstate::responsive;
                timer_.expires_after(std::chrono::seconds(TIMEOUT));
                // process request and send out response
                std::string response = dispatcher.dispatch(request_str);
                ws_.async_write(boost::asio::buffer(response), yield);
            }
        }
        // unrecoverable error
        catch (std::exception &e) {
            if (processor.get_session().userid.has_value()) {
                nibaserver::db_accessor::logout(processor.get_session().userid.value());
            }
            close_down_ = true;
            timer_.cancel();
            BOOST_LOG_SEV(logger_, sev::info) << "Session ending, reason: " << e.what();
        }
    });

    auto self2(shared_from_this());
    boost::asio::spawn(strand_, [this, self2](boost::asio::yield_context yield) {
        for (;;) {
            if (close_down_)
                break;
            // check it has expired
            if (timer_.expiry() <= std::chrono::steady_clock::now()) {
                if (ws_.is_open() && ping_state_ == pingstate::responsive) {
                    ping_state_ = pingstate::onhold;
                    timer_.expires_after(std::chrono::seconds(TIMEOUT));
                    // Now send the ping
                    boost::system::error_code ec;
                    ws_.async_ping({}, yield[ec]);
                    // we don't care about ec here
                } else if (ws_.is_open()) {
                    // ping state is onhold - no activity in last 15 seconds
                    BOOST_LOG_SEV(logger_, sev::info) << "Connection closing due to ping timeout";
                    ws_.next_layer().next_layer().cancel();
                    break;
                }
            }
            // wait on the timer
            boost::system::error_code ec;
            timer_.async_wait(yield[ec]);
            if (ec && ec != boost::asio::error::operation_aborted)
                break;
        }
        timer_.cancel();
        BOOST_LOG_SEV(logger_, sev::info) << "ping coroutine exiting";
    });
}
