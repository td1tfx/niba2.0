#include "Listener.h"



Listener::Listener(boost::asio::io_context& ioc, ssl::context& ctx,	tcp::endpoint endpoint) : ctx_(ctx), acceptor_(ioc), socket_(ioc)
{
	boost::system::error_code ec;

	// Open the acceptor
	acceptor_.open(endpoint.protocol(), ec);
	if (ec)
	{
		fail(ec, "open");
		return;
	}

	// Allow address reuse
	acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
	if (ec)
	{
		fail(ec, "set_option");
		return;
	}

	// Bind to the server address
	acceptor_.bind(endpoint, ec);
	if (ec)
	{
		fail(ec, "bind");
		return;
	}

	// Start listening for connections
	acceptor_.listen(
		boost::asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		fail(ec, "listen");
		return;
	}
}


Listener::~Listener()
{
}


// Start accepting incoming connections
void Listener::run()
{
	if (!acceptor_.is_open())
		return;
	doAccept();
}

void Listener::doAccept()
{
	acceptor_.async_accept(
		socket_,
		std::bind(
			&Listener::onAccept,
			shared_from_this(),
			std::placeholders::_1));
}

void Listener::onAccept(boost::system::error_code ec)
{
	if (ec)
	{
		fail(ec, "accept");
	}
	else
	{
		// Create the session and run it
		std::make_shared<Session>(std::move(socket_), ctx_)->run();
	}

	// Accept another connection
	doAccept();
}