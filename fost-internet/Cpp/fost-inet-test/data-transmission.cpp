/*
    Copyright 2011, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"


using namespace fostlib;


FSL_TEST_SUITE( data_transmission );


namespace {
    void perform_hash() {
        boost::asio::io_service service;
        boost::asio::ip::tcp::acceptor server(
            service, boost::asio::ip::tcp::endpoint(
                host("0.0.0.0").address(), uint16_t(6218)));
        std::auto_ptr< boost::asio::ip::tcp::socket > sock(
            new boost::asio::ip::tcp::socket( service ));
        server.accept(*sock);
    }
}

FSL_TEST_FUNCTION( large_send ) {
    worker server;
    server(perform_hash);
    network_connection cnx(host("localhost"), 6218);
}
