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


    /// TCP connection
    class rask_tcp : public rask_connection<boost::asio::ip::tcp::socket> {
    protected:
        rask_tcp(boost::asio::io_service &ios)
        : rask_connection<boost::asio::ip::tcp::socket>(server_side), socket(ios) {
        }

        boost::asio::io_service &get_io_service() override {
            return socket.get_io_service();
        }

        /// Loop for processing inbound packets
        void process_inbound(boost::asio::yield_context &) override;

    public:
        /// The socket for this connection
        boost::asio::ip::tcp::socket socket;
    };


    /// Counters used for TCP
    extern rask_counters rask_tcp_counters;

    /// Listen for a connection. When a connection is established the factory
    /// is used to create a connection object which is then expected to handle
    /// the ongoing connection.
    void tcp_listen(boost::asio::io_service &ios, fostlib::host netloc,
        std::function<std::shared_ptr<rask_tcp>(boost::asio::ip::tcp::socket)> factory);


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
