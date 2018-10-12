//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_EXAMPLE_COMMON_SERVER_CERTIFICATE_HPP
#define BOOST_BEAST_EXAMPLE_COMMON_SERVER_CERTIFICATE_HPP

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <cstddef>
#include <memory>

/*  Load a signed certificate into the ssl context, and configure
    the context for use with a server.

    For this to work with the browser or operating system, it is
    necessary to import the "Beast Test CA" certificate into
    the local certificate store, browser, or operating system
    depending on your environment Please see the documentation
    accompanying the Beast certificate for more details.
*/
inline
void
load_server_certificate(boost::asio::ssl::context& ctx)
{
    /*
        The certificate was generated from CMD.EXE on Windows 10 using:

        winpty openssl dhparam -out dh.pem 2048
        winpty openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 10000 -out cert.pem -subj "//C=US\ST=CA\L=Los Angeles\O=Beast\CN=www.example.com"
    */

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


    std::string const key =
R"(-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEA0U9vp6mOrjUPVLhYAKFpREoR/+CieVGDKUghnNV11TOt66fU
fwhzRirU9OmlytzDLPgmY3rGTdc0B6pY6FJGmGy6SI2RCxfmRrBh1Sa2TaFZN/3u
iNQym+eR3lUGRq5c2XGB/re6gnRKidOL3wX+N3ZcPxkxBEO3DNSWq9JqL8NGWASE
f+xfMo09AiXnX7WZQcKscJqXVU2KwpJOvxHT7Yt928nuX84sDXWCi63oRIunI9Z/
2bp9L0hsrTNAUrKIMES6OXU5vCyXsPpIAbWv61IriCbjLr3oewt6dlj7tQdT0OXa
wbYxjnyFIEfHBXmC3I+K4hu3Y2nf1zJOBvHglQIDAQABAoIBAEWpnou6zxYD6tpT
kFaFYjesP1leNjUSgAjZTPaZxOH0lUn9TagJ8/PChaYNrXRbDUqGPKCXWDDIEzUJ
6RVJbN2GMmm7VC7RAX89bRI4FCy6KquMBDveBAoHhHDu82vK1HQ6Zv2TwKtycOx2
T/MmV3m2FvWZOMsmjmHiz1LYcSb7yNB3Z7bV/TKzI3NQHgvs00aduYYMa9oLn/UK
pzF+zeP7ZGGpxdsPWi5NTHvQ0mY0ox1FKI3fnR2QUwG4T3shVBtXWjQ3yRS9dUi2
bnHZfSWmm7ZnQLE9kd86gFkBUd3BirE2WfPxj5GpTqyzJZe1mRhftLRCUI6MW7I4
qIs9ifECgYEA8YGi5iUTN/9cSYsQOJkkyoQWU2Ue0QqnM23VCUs1CRpjwKkJlnxS
3qOppDTjD/87P+Oq3HvC3EAHeN6glQ1Gq0xQSbYAv81HY6rlElHqtmVb59lMZxkv
30iAXSWfC8enXV4TU3xaY6jhghAHXS/oerdI0uSOaSBa0qRpEHzNns8CgYEA3d8o
Y3UeThxTAlGIT3k1+agc7JnNiIcfiKoCv+M+TsJVUemtVeg1hm8bgRcGBL2aSCZM
ttB32g4IqBrehUxEG6Z2CoDlgjCLOs44PeYoaQgZ1XzGBs5DZrSs08F4+fAOyWT1
+5/so++MREu2LPU30uKOQLMPJojzm766156iA1sCgYBwua/NeLS1yI3NDgnuQR05
IdpB5ssxi6VrjfKIOgBxVU4GWWBOS41quj2ulbj+eIWxVpUkAOh9Ya2bL18I839u
6Vn4bel/nVObI+N0X6QMqtbxKy5MWSdNM0wnLi5Ek1Z9cDHZIAwYxHUt/klRp9D9
pptpRKFsVFvQQkmAFqDrZQKBgQCvkM1x3hRL9OfOltvJH34RfvDcXFwiY+IVD4q7
csM0KVzAQ6uwpd3s8fCOtrxBdhMmcoATzWR5fM9mzlQ0oIPoaVFQwGxcF7Okpdgl
SwHBEvL2VxSQABmfpOp5acgj5sycmrJ3GTZ1BrI0vMB8VNKGolW0ZrxfXtF99W/9
/P3wrwKBgQCMu27YJMk21rM+sQuBisPt0lO7nOx3KfFgN2KAXVxZp9jiYyAkNwrB
oL99f3h0IiKFVbWKLcE6V4LUVFvvY1WebyO37LOdzYv+4r4Af4uoPbDGqc75pmBx
d80BojEu/Fcy0RXblovxmxin/NKdftS1bYytMgeb2e5ndPSia3SVUg==
-----END RSA PRIVATE KEY-----)";

    std::string const dh =
R"(-----BEGIN DH PARAMETERS-----
MIGHAoGBANZaLAnxkSdgP4Fh4D3umehbjEWSw46HAIOTnqz4pP0VERvMUVevDDVI
gb5baHyfcW3tzijkQ3klN/y0u6hqxFu43BKFfVuSXc5FPEi6DwDpCJ4S2aPQksD9
7N2oAUiNkHlbGhnThIeMRz2HBXMBydhZVuSiRvBiqZsB6PRYxF4rAgEC
-----END DH PARAMETERS-----)";

    ctx.set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2 |
        boost::asio::ssl::context::single_dh_use);

    ctx.use_certificate_chain(
        boost::asio::buffer(cert.data(), cert.size()));

    ctx.use_private_key(
        boost::asio::buffer(key.data(), key.size()),
        boost::asio::ssl::context::file_format::pem);

    ctx.use_tmp_dh(
        boost::asio::buffer(dh.data(), dh.size()));
}

#endif