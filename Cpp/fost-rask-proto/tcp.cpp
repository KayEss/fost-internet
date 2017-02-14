/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/rask/tcp.hpp>


const fostlib::module rask::c_rask_proto_tcp(c_rask_proto, "tcp");


void rask::tcp_listen(
    boost::asio::io_service &ios, fostlib::host netloc,
    std::function<std::shared_ptr<tcp_connection>(boost::asio::ip::tcp::socket)> factory
) {
}
