#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/stream.hpp>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

												// Report a failure
static void fail(boost::system::error_code ec, char const* what)
{
	std::cerr << what << ": " << ec.message() << "\n";
}

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(tcp::socket socket, ssl::context& ctx);
	~Session();

	void run();
	void onHandshake(boost::system::error_code ec);
	void onAccept(boost::system::error_code ec);
	void doRead();
	void onRead(boost::system::error_code ec, std::size_t bytes_transferred);
	void onWrite(boost::system::error_code ec, std::size_t bytes_transferred);

private:
	tcp::socket socket_;
	websocket::stream<ssl::stream<tcp::socket&>> ws_;
	boost::asio::strand<boost::asio::io_context::executor_type> strand_;
	boost::beast::multi_buffer buffer_;
};

