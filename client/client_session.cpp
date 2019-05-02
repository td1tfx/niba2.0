#include "client_session.h"

#include <boost/algorithm/string.hpp>

namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>

using namespace nibaclient;

nibaclient::client_session::client_session(std::string const &host, std::string const &port,
                                           boost::asio::io_context &ioc, ssl::context &ssl_ctx) :
    host_{host},
    port_{port}, ioc_{ioc}, resolver_{ioc_}, ws_{ioc_, ssl_ctx}, ready_promise_{},
    ready_future_{ready_promise_.get_future()} {}

void nibaclient::client_session::start() {
    // Note we only have 1 ioc_ thread, so no strand needed
    boost::asio::spawn(ioc_, [this, self = shared_from_this()](boost::asio::yield_context yield) {
        try {
            auto const results = resolver_.async_resolve(host_, port_, yield);
            beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
            beast::get_lowest_layer(ws_).async_connect(results, yield);
            boost::asio::ip::tcp::no_delay option(true);
            beast::get_lowest_layer(ws_).socket().set_option(option);
            beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
            ws_.next_layer().async_handshake(ssl::stream_base::client, yield);
            beast::get_lowest_layer(ws_).expires_never();
            ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
            ws_.async_handshake(host_, "/", yield);
            ready_promise_.set_value();
        } catch (...) {
            // Apparently this can throw as well, but let's see if we care...
            ready_promise_.set_exception(std::current_exception());
            return;
        }
        // Now start long running read coroutine
        for(;;) {
            std::string incoming_str;
            auto buffer = boost::asio::dynamic_buffer(incoming_str);
            try {
                ws_.async_read(buffer, yield);
                std::cerr << request_stopwatch_.elapsed_ms() << std::endl;
                std::cout << incoming_str << "\n";
                auto json = nlohmann::json::parse(incoming_str);
                if (json.find("error") != json.end()) {
                    std::string err = json["error"].get<std::string>();
                    throw std::runtime_error(err.c_str());
                }
                nibashared::message::tag tag{json.at("tag").get<int>()};
                if (request_.has_value() && tag == nibashared::message::tag::response) {
                    // Since we strictly wait for 1 response per request, this response must match the request.
                    // Merge it with the request and process.
                    std::visit([&json, this, yield](auto& response) {
                        processor_.process_response(response, json);
                        // Manually chain the getdata message
                        if (response.type == nibashared::message::type::login) {
                            write_message(nibashared::message_getdata{}, yield);
                        } else {
                            // Set the promise!
                            std::cout << "DEBUG setting promise for response\n";
                            request_promise_.set_value(response.type);
                        }
                    }, request_.value());
                } else {
                    // It's some unknown message, just process it
                    nibashared::message::dispatcher(json, [this](auto message){
                        // Incoming message, assume "it just works"
                        processor_(message);
                    });
                }
            } catch (std::exception& ex) {
                std::cout << "failed to process incoming message: " << ex.what() << "\n";
                break;
            }
        }

        // processor_.dispatch(message, response_str);
    });
}

void nibaclient::client_session::block_until_ready() { ready_future_.get(); }

void nibaclient::client_session::stop() {
    // Exceptions may happen in the destructor, but if it throws, then let it crash...?
    if (ws_.is_open()) {
        // ioc should finish the close before shutting down
        ws_.async_close(websocket::close_code::normal, [](beast::error_code ec) {
            std::cout << ec.message() << std::endl;
        });
    }
}

std::future<nibashared::message::type>
nibaclient::client_session::handle_cmd(const std::string &input) {
    try {
        handled_ = true;
        std::vector<std::string> results;
        boost::split(results, input, boost::is_any_of("\t "));
        if (results.empty())
            throw std::runtime_error("empty input");
        // Dynamically sized arguments
        if (results[0] == "reordermagic") {
            std::vector<int> selected;
            std::for_each(results.begin() + 1, results.end(),
                          [&selected](auto &magic_id) { selected.push_back(std::stoi(magic_id)); });
            return create_and_go<nibashared::message_reordermagic>(std::move(selected));
        }
        if (results.size() == 3) {
            if (results[0] == "register") {
                return create_and_go<nibashared::message_registration>(std::move(results[1]),
                                                                   std::move(results[2]));
            } else if (results[0] == "login") {
                return create_and_go<nibashared::message_login>(std::move(results[1]),
                                                                std::move(results[2]));
                // Force a getdata after login
                // create_and_go<nibashared::message_getdata>();
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
                // This should be posted to ioc, TODO
                std::this_thread::sleep_for(std::chrono::seconds(std::stoi(results[1])));
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
        } else {
            throw std::runtime_error("incorrect command");
        }
    } catch (std::exception &exc) {
        std::cout << exc.what() << std::endl;
    }
    handled_ = false;
    // Return a dummy future, that has promised already fulfilled
    std::promise<nibashared::message::type> promise;
    auto future = promise.get_future();
    promise.set_value(nibashared::message::type::none);
    return future;
}

std::chrono::high_resolution_clock::time_point nibaclient::client_session::earliest() const {
    if (!handled_)
        return {};
    // If no change then it means do whatever
    if (processor_.get_session().earliest_time == processor_.get_session().current_time)
        return {};
    return processor_.get_session().earliest_time;
}
