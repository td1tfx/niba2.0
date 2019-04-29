#include "client_session.h"

#include <boost/algorithm/string.hpp>

namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>

using namespace nibaclient;

namespace {

// auto set a void promise on destruction
struct promise_setter {
public:
    promise_setter(std::promise<void>& promise): promise_(promise) {}
    ~promise_setter() {
        promise_.set_value();
    }
private:
    std::promise<void>& promise_;
};

}

// , work_(ioc_)
nibaclient::client_session::client_session(std::string const &host, std::string const &port,
                                           boost::asio::io_context &ioc, ssl::context &ssl_ctx) :
    host_(host),
    port_(port), ioc_(ioc), resolver_(ioc_), ws_(ioc_, ssl_ctx) {
    // TODO make all async
    // everything can be sync because this client has no ui anyway
    auto const results = resolver_.resolve(host_, port_);
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
    beast::get_lowest_layer(ws_).connect(results);
    boost::asio::ip::tcp::no_delay option(true);
    beast::get_lowest_layer(ws_).socket().set_option(option);
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
    ws_.next_layer().handshake(ssl::stream_base::client);
    beast::get_lowest_layer(ws_).expires_never();
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));    
    ws_.handshake(host_, "/");
}

nibaclient::client_session::~client_session() { close(); }

void nibaclient::client_session::close() {
    if (ws_.is_open()) {
        beast::get_lowest_layer(ws_).cancel();
        ws_.close(websocket::close_code::normal);
    }
}

void nibaclient::client_session::handle_cmd(const std::string &input, std::promise<void>&& promise) {
    promise_setter set_promise(promise);
    try {
        handled_ = true;
        std::vector<std::string> results;
        boost::split(results, input, boost::is_any_of("\t "));
        if (results.empty())
            throw std::runtime_error("empty input");
        // dynamic sized
        if (results[0] == "reordermagic") {
            std::vector<int> selected;
            std::for_each(results.begin() + 1, results.end(),
                          [&selected](auto &magic_id) { selected.push_back(std::stoi(magic_id)); });
            return create_and_go<nibashared::message_reordermagic>(std::move(selected));
        }
        if (results.size() == 3) {
            if (results[0] == "register") {
                return create_and_go<nibashared::message_register>(std::move(results[1]),
                                                                   std::move(results[2]));
            } else if (results[0] == "login") {
                create_and_go<nibashared::message_login>(std::move(results[1]),
                                                         std::move(results[2]));
                // force a getdata after login
                return create_and_go<nibashared::message_getdata>();
            } else if (results[0] == "fusemagic") {
                return create_and_go<nibashared::message_fusemagic>(std::stoi(results[1]),
                                                                    std::stoi(results[2]));
            }
        } else if (results.size() == 2) {
            if (results[0] == "fight") {
                return create_and_go<nibashared::message_fight>(std::stoi(results[1]));
            } else if (results[0] == "learnmagic") {
                return create_and_go<nibashared::message_learnmagic>(std::stoi(results[1]));
            } else if (results[0] == "timeout") {
                std::this_thread::sleep_for(std::chrono::seconds(std::stoi(results[1])));
                return;
            }
        } else if (results.size() == 7) {
            if (results[0] == "create") {
                return create_and_go<nibashared::message_createchar>(
                    nibashared::player{.name = results[1],
                                       .gender = results[2].at(0),
                                       .attrs = {.strength = std::stoi(results[3]),
                                                 .dexterity = std::stoi(results[4]),
                                                 .physique = std::stoi(results[5]),
                                                 .spirit = std::stoi(results[6])}});
            }
        }
    } catch (...) {
        std::cout << "incorrect command" << std::endl;
        // parsing failure whatever
    }
    handled_ = false;
}

std::chrono::high_resolution_clock::time_point nibaclient::client_session::earliest() const {
    if (!handled_)
        return {};
    // if no change then it means do whatever
    if (processor_.get_session().earliest_time == processor_.get_session().current_time)
        return {};
    return processor_.get_session().earliest_time;
}
