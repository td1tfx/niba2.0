#include "server_session.h"
#include "db_accessor.h"
#include "server_processor.h"
#include "util.h"

#include <boost/asio/spawn.hpp>
#include <boost/beast/http.hpp>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace sev = boost::log::trivial;

namespace nibaserver {

server_session::server_session(boost::asio::io_context &ioc, boost::asio::ip::tcp::socket &&socket,
                               boost::asio::ssl::context &ctx, nibaserver::db_accessor &&db,
                               session_map &ss_map) :
    ioc_{ioc},
    strand_{ioc_}, ws_{std::move(socket), ctx}, db_{std::move(db)}, ss_map_{ss_map} {}

server_session::~server_session() { BOOST_LOG_SEV(logger_, sev::info) << "Session destructed"; }

void server_session::go() {
    auto self(shared_from_this());
    boost::asio::spawn(strand_, [this, self](boost::asio::yield_context yield) {
        server_processor processor(yield, db_, ss_map_, self);
        try {
            boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
            ws_.next_layer().async_handshake(ssl::stream_base::server, yield);
            boost::beast::get_lowest_layer(ws_).expires_never();
            ws_.set_option(
                websocket::stream_base::timeout::suggested(boost::beast::role_type::server));
            // Accept the websocket handshake
            ws_.set_option(websocket::stream_base::decorator([](websocket::response_type &res) {
                // might only need one
                res.set(http::field::server, "niba-server");
                res.insert(http::field::sec_websocket_protocol, "niba-server");
            }));
            ws_.async_accept(yield);
            for (;;) {
                // recv request
                std::string request_str;
                auto buffer = boost::asio::dynamic_buffer(request_str);
                nibautil::stopwatch stopwatch_next_request;
                ws_.async_read(buffer, yield);
                BOOST_LOG_SEV(logger_, sev::info)
                    << "idled for " << stopwatch_next_request.elapsed_ms() << "ms";
                // check how long the request itself is processed
                nibautil::stopwatch stopwatch;
                // process request and send out response
                std::string response = processor.dispatch(request_str);
                write(std::move(response));
                BOOST_LOG_SEV(logger_, sev::info)
                    << "request processed in " << stopwatch.elapsed_ms() << "ms";
            }
        }
        // unrecoverable error
        catch (std::exception &e) {
            // moving this logging line to the end will cause crashes
            BOOST_LOG_SEV(logger_, sev::info) << "Session ending, reason: " << e.what();
            if (processor.get_session().userid) {
                db_.logout(*processor.get_session().userid, yield);
            }
        }
    });
}

void server_session::write(std::string str) {
    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "write called";
    boost::asio::spawn(strand_, [this, str{std::move(str)},
                                    self{shared_from_this()}](boost::asio::yield_context yield) {
        write_queue_.emplace(std::move(str));
        BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "str enqueued";
        if (write_queue_.size() != 1) {
            return;
            // Don't do anything, someone will handle it
        }
        for (;;) {
            if (write_queue_.empty()) {
                return;
            }
            // write it
            ws_.async_write(boost::asio::buffer(write_queue_.front()), yield);
            // pop after we are done
            write_queue_.pop();
        }
    });
}

} // namespace nibaserver
