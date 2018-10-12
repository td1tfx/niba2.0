#include "Session.h"



Session::Session(tcp::socket socket, ssl::context& ctx) : socket_(std::move(socket)), ws_(socket_,ctx),strand_(ws_.get_executor())
{
}


Session::~Session()
{
}

void Session::run() {
	// Perform the SSL handshake
	ws_.next_layer().async_handshake(
		ssl::stream_base::server,
		boost::asio::bind_executor(
			strand_,
			std::bind(
				&Session::onHandshake,
				shared_from_this(),
				std::placeholders::_1)));

}

void Session::onHandshake(boost::system::error_code ec)
{
	if (ec)
		return fail(ec, "handshake");

	// Accept the websocket handshake
	ws_.async_accept(
		boost::asio::bind_executor(
			strand_,
			std::bind(
				&Session::onAccept,
				shared_from_this(),
				std::placeholders::_1)));
}

void Session::onAccept(boost::system::error_code ec)
{
	if (ec)
		return fail(ec, "accept");

	// Read a message
	doRead();
}

void Session::doRead()
{
	// Read a message into our buffer
	ws_.async_read(
		buffer_,
		boost::asio::bind_executor(
			strand_,
			std::bind(
				&Session::onRead,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2)));
}

void Session::onRead(
	boost::system::error_code ec,
	std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	// This indicates that the session was closed
	if (ec == websocket::error::closed)
		return;

	if (ec)
		fail(ec, "read");

	// Echo the message
	ws_.text(ws_.got_text());
	ws_.async_write(
		buffer_.data(),
		boost::asio::bind_executor(
			strand_,
			std::bind(
				&Session::onWrite,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2)));
}

void Session::onWrite(boost::system::error_code ec,	std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	if (ec)
		return fail(ec, "write");

	// Clear the buffer
	buffer_.consume(buffer_.size());

	// Do another read
	doRead();
}
