/*
    Copyright 2016-2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/internet>
#include <fost/rask/counters.hpp>
#include <fost/rask/connection.hpp>
#include <fost/rask/packet.hpp>

#include <boost/asio/spawn.hpp>


namespace fostlib {


    /// Module for TCP
    extern const module c_rask_proto_tcp;


    using rask_tcp_server = rask_server<boost::asio::ip::tcp::socket>;
    using rask_tcp = rask_connection<boost::asio::ip::tcp::socket>;


    /// Counters used for TCP
    extern rask_counters rask_tcp_counters;


}


/// Implementation of TCP data send for outbound packets
template<> inline
void rask::out_packet::operator () (
    boost::asio::ip::tcp::socket &cnx, boost::asio::yield_context &yield
) const {
    boost::asio::streambuf header;
    size_sequence(size(), header);
    header.sputc(control);
    std::array<boost::asio::streambuf::const_buffers_type, 2>
        data{{header.data(), buffer->data()}};
    async_write(cnx, data, yield);
}

