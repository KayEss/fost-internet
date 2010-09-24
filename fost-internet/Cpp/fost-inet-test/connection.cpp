/*
    Copyright 2010, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/internet>


using namespace fostlib;


FSL_TEST_SUITE( connection );


FSL_TEST_FUNCTION( connect ) {
    FSL_CHECK_EXCEPTION(
        network_connection cnx(host("localhost"), 64545),
        exceptions::connect_failure&);
    FSL_CHECK_EXCEPTION(
        network_connection cnx(host("1.1.1.1"), 64545),
        exceptions::connect_failure&);
}


FSL_TEST_FUNCTION( read_timeouts ) {
    boost::asio::io_service service;
    host localhost;
    uint16_t port = 64544u;

    // Set up a server on a socket
    boost::asio::ip::tcp::acceptor server(service,
        boost::asio::ip::tcp::endpoint(localhost.address(), port));

    // Connect to it and try to read from it
    network_connection cnx(localhost, port);

    utf8_string s;
    FSL_CHECK_EXCEPTION(cnx >> s,
        fostlib::exceptions::not_implemented&);

    std::vector< unsigned char > data(256);
    FSL_CHECK_EXCEPTION(cnx >> data,
        fostlib::exceptions::not_implemented&);
}
