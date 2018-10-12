//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket SSL server, coroutine
//
//------------------------------------------------------------------------------

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "Listener.h"
#include "cert_loader.hpp"



//------------------------------------------------------------------------------


int main(int argc, char* argv[])
{


	auto const address = boost::asio::ip::make_address("0.0.0.0");
	auto const port = static_cast<unsigned short>(19999);
	auto const threads = std::max<int>(1, 1);

	// The io_context is required for all I/O
	boost::asio::io_context ioc{ threads };

	// The SSL context is required, and holds certificates
	ssl::context ctx{ ssl::context::sslv23 };

	// This holds the self-signed certificate used by the server
	load_server_certificate(ctx);

	// Create and launch a listening port
	std::make_shared<Listener>(ioc, ctx, tcp::endpoint{ address, port })->run();

	// Run the I/O service on the requested number of threads
	std::vector<std::thread> v;
	v.reserve(threads - 1);
	for (auto i = threads - 1; i > 0; --i)
		v.emplace_back(
			[&ioc]
	{
		ioc.run();
	});
	ioc.run();

	return EXIT_SUCCESS;
}