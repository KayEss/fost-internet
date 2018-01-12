/*
    Copyright 2010-2018, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/internet>

#include <boost/timer.hpp>
#include <thread>


using namespace fostlib;


FSL_TEST_SUITE( connection );


FSL_TEST_FUNCTION( connect_failure ) {
    setting< int64_t > c_connect_timeout(
        "fost-internet/Cpp/fost-inet-test/connection.cpp",
        "Network settings", "Connect time out", 1);
    setting< int64_t > c_read_timeout(
        "fost-internet/Cpp/fost-inet-test/connection.cpp",
        "Network settings", "Read time out", 1);
    setting< int64_t > c_write_timeout(
        "fost-internet/Cpp/fost-inet-test/connection.cpp",
        "Network settings", "Write time out", 1);
    FSL_CHECK_EXCEPTION(
        network_connection(host("localhost"), 64545) << "Data\n",
        exceptions::socket_error&);
    FSL_CHECK_EXCEPTION(
        network_connection(host("10.45.234.124"), 64545) << "Data\n",
        exceptions::socket_error&);
}


FSL_TEST_FUNCTION( read_timeouts ) {
    boost::asio::io_service service;
    host localhost;
    uint16_t port = 64544u;
    // Set a very short time out whilst running the test
    const setting< int64_t > c_read_timeout(
        "fost-internet/Cpp/fost-inet-test/connection.cpp",
        "Network settings", "Read time out", 1);

    // Set up a server on a socket
    boost::asio::ip::tcp::acceptor server(service,
        boost::asio::ip::tcp::endpoint(localhost.address(), port));

    // Connect to it and try to read from it
    {
        network_connection cnx(localhost, port);
        utf8_string s;
        FSL_CHECK_EXCEPTION(cnx >> s,
            fostlib::exceptions::read_timeout&);
    }
    {
        network_connection cnx(localhost, port);
        std::vector< unsigned char > data(256);
        FSL_CHECK_EXCEPTION(cnx >> data,
            fostlib::exceptions::read_timeout&);
    }
}


namespace {
    void send_data() {
        auto service = std::make_unique<boost::asio::io_service>();
        host localhost;
        uint16_t port = 64543u;
        // Set up a server on a socket
        boost::asio::ip::tcp::acceptor server(*service,
            boost::asio::ip::tcp::endpoint(localhost.address(), port));
        auto sock = std::make_unique<boost::asio::ip::tcp::socket>(*service);
        // Accept the connection
        server.accept(*sock);
        network_connection server_cnx(std::move(service), std::move(sock));
        // Send a few KB of data
        std::string data(10240, '*');
        server_cnx << data;
    }
}
FSL_TEST_FUNCTION( early_closure ) {
    boost::timer timer;
    boost::asio::io_service service;
    host localhost;
    uint16_t port = 64543u;
    // Send some data to the socket
    worker server;
    server(send_data);
    // Wait for long enough for the server to start
    std::this_thread::sleep_for(std::chrono::milliseconds{250});
    // Open a connection to the server
    network_connection client(localhost, port);
    // Try to read more data than the server is going to send before it closes the connection
    std::vector<unsigned char> data(20480);
    FSL_CHECK_EXCEPTION(client >> data, fostlib::exceptions::unexpected_eof&);
    FSL_CHECK(timer.elapsed() < 1);
}
