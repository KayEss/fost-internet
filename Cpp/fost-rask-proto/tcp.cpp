/**
    Copyright 2017-2018, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <fost/rask/tcp.hpp>


const fostlib::module fostlib::hod::c_rask_proto_tcp(c_rask_proto, "tcp");


void fostlib::hod::tcp_listen(
    boost::asio::io_service &ios, fostlib::host netloc,
    std::function<std::shared_ptr<tcp_connection>(boost::asio::ip::tcp::socket)> factory
) {
}
