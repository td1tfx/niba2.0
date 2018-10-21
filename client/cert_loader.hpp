//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_EXAMPLE_COMMON_ROOT_CERTIFICATES_HPP
#define BOOST_BEAST_EXAMPLE_COMMON_ROOT_CERTIFICATES_HPP

#include <boost/asio/ssl.hpp>
#include <string>

namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>

namespace detail {

// The template argument is gratuituous, to
// allow the implementation to be header-only.
//
template<class = void>
void load_root_certificates(ssl::context &ctx, boost::system::error_code &ec) {

    // https://stackoverflow.com/questions/6452756/exception-running-boost-asio-ssl-example

    std::string const cert =
        R"(-----BEGIN CERTIFICATE-----
MIICtDCCAZwCCQC82WtqGSF5jDANBgkqhkiG9w0BAQsFADAcMQswCQYDVQQGEwJV
UzENMAsGA1UECgwEbmliYTAeFw0xODEwMTIwNDM1MThaFw0yODEwMDkwNDM1MTha
MBwxCzAJBgNVBAYTAlVTMQ0wCwYDVQQKDARuaWJhMIIBIjANBgkqhkiG9w0BAQEF
AAOCAQ8AMIIBCgKCAQEA0U9vp6mOrjUPVLhYAKFpREoR/+CieVGDKUghnNV11TOt
66fUfwhzRirU9OmlytzDLPgmY3rGTdc0B6pY6FJGmGy6SI2RCxfmRrBh1Sa2TaFZ
N/3uiNQym+eR3lUGRq5c2XGB/re6gnRKidOL3wX+N3ZcPxkxBEO3DNSWq9JqL8NG
WASEf+xfMo09AiXnX7WZQcKscJqXVU2KwpJOvxHT7Yt928nuX84sDXWCi63oRIun
I9Z/2bp9L0hsrTNAUrKIMES6OXU5vCyXsPpIAbWv61IriCbjLr3oewt6dlj7tQdT
0OXawbYxjnyFIEfHBXmC3I+K4hu3Y2nf1zJOBvHglQIDAQABMA0GCSqGSIb3DQEB
CwUAA4IBAQBdpha/n2zK5gLjksIkAR6zhEk6EUsRoIPjd2sVspYFVHajMcfb8WTJ
a97NDCETnc+vRakVy3lXWNwp66YaT92q7Ab2Ci4nDE686FK6T0E6BcNAD4HWKTLI
7ilCS7ECcQwdUgwlXh6PFr5ZhTv/vqiW3yZbx1o4CR1NkSBO+g7YcBs4c3ZldSGQ
BLuPCiUIlExaWFDZ80dC9qEYXoj+aImQbBCc5EM0Zzk90/k4NImuFBBMDeh9V/MG
luOLJAzTRGJYImdk5DPBV9mp0K+HtA1+gIxb2AcFadXFfBGgiZKiZ+MWiRJx9X5+
UaAKuOzw5yef9xSK6Ek2VdCtfvycTufe
-----END CERTIFICATE-----)";

    ctx.add_certificate_authority(boost::asio::buffer(cert.data(), cert.size()), ec);
    if (ec)
        return;
}

} // namespace detail

// Load the root certificates into an ssl::context
//
// This function is inline so that its easy to take
// the address and there's nothing weird like a
// gratuituous template argument; thus it appears
// like a "normal" function.
//

inline void load_root_certificates(ssl::context &ctx, boost::system::error_code &ec) {
    detail::load_root_certificates(ctx, ec);
}

inline void load_root_certificates(ssl::context &ctx) {
    boost::system::error_code ec;
    detail::load_root_certificates(ctx, ec);
    if (ec)
        throw boost::system::system_error{ec};
}

#endif