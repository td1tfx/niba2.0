#pragma once

#include "Session.h"

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>


class Listener : public std::enable_shared_from_this<Listener>
{
public:
	Listener(boost::asio::io_context& ioc,
		ssl::context& ctx,
		tcp::endpoint endpoint);

	~Listener();

	void run();
	void doAccept();
	void onAccept(boost::system::error_code ec);

private:
	ssl::context& ctx_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

