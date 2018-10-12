#pragma once



/*


class Server : public std::enable_shared_from_this<Server>
{
public:
	Server();
	~Server();

	// Accepts incoming connections and launches the sessions
	void doListen(boost::asio::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, boost::asio::yield_context yield);
	
private:
	// Report a failure
	void fail(boost::system::error_code ec, char const* what) {	
		std::cerr << what << ": " << ec.message() << "\n";
	}
	// Echoes back all received WebSocket messages
	void doSession_(tcp::socket& socket, ssl::context& ctx, boost::asio::yield_context yield);
};

*/