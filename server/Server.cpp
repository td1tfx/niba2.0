#include "Server.h"

/*

Server::Server()
{
}


Server::~Server()
{
}

void Server::doListen(boost::asio::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, boost::asio::yield_context yield) 
{
	boost::system::error_code ec;


	//open the acceptor
	tcp::acceptor acceptor(ioc);
	acceptor.open(endpoint.protocol(), ec);
	if (ec) {
		return fail(ec, "open");
	}

	//allow address reuse
	acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
	if (ec) {
		return fail(ec, "bind");
	}

	//start listening for connections,
	acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
	if (ec)
		return fail(ec, "listen");
	for (;;) 
	{
		tcp::socket socket(ioc);
		acceptor.async_accept(socket, yield[ec]);
		if (ec) 
		{
			fail(ec, "accept");
		}
		else {
			boost::asio::spawn(
				acceptor.get_executor().context(),
				std::bind(
					&Server::doSession_,
					std::move(socket),
					std::ref(ctx),
					std::placeholders::_1
				)
			);
		}
	
	}

}

void Server::doSession_(tcp::socket& socket, ssl::context& ctx, boost::asio::yield_context yield) {
	boost::system::error_code ec;

	//Construct the stream by moving in the socket
	websocket::stream<ssl::stream<tcp::socket&>> ws{ socket, ctx };

	//Perform the SSL handshake
	ws.next_layer().async_handshake(ssl::stream_base::server, yield[ec]);
	if (ec) 
	{
		return fail(ec, "handshake");
	}

	//Accept the websocket handshake
	ws.async_accept(yield[ec]);
	if (ec) {
		return fail(ec, "accept");
	}

	for (;;) 
	{
		//This buffer will hold the incoming message
		boost::beast::multi_buffer buffer;

		//Read a messgae
		ws.async_read(buffer, yield[ec]);

		//This indicates that the session was closed
		if (ec == websocket::error::closed) 
		{
			break;
		}

		if (ec) 
		{
			return fail(ec, "read");
		}

		//Echo the message back
		ws.text(ws.got_text());
		ws.async_write(buffer.data(), yield[ec]);
		if (ec) 
		{
			return fail(ec, "write");
		}

	}
}

*/