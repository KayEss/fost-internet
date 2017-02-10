/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/rask/tcp.hpp>


const fostlib::module fostlib::c_rask_proto_tcp(c_rask_proto, "tcp");


fostlib::rask_counters fostlib::rask_tcp_counters(c_rask_proto_tcp);


void fostlib::tcp_listen(
    boost::asio::io_service &ios, fostlib::host netloc,
    std::function<std::shared_ptr<fostlib::rask_tcp>(boost::asio::ip::tcp::socket)> factory
) {
}


void fostlib::rask_tcp::process_inbound(boost::asio::yield_context &yield) {
    boost::asio::streambuf input_buffer;
    auto decode{rask::make_decoder(
        [&]() {
            boost::asio::async_read(socket, input_buffer,
                boost::asio::transfer_exactly(1), yield);
            return input_buffer.sbumpc();
        },
        [&](char *into, std::size_t bytes) {
            boost::asio::async_read(socket, input_buffer,
                boost::asio::transfer_exactly(bytes), yield);
            return input_buffer.sgetn(into, bytes);
        })};
    while ( socket.is_open() ) {
        try {
            std::size_t packet_size = decode.read_size();
            uint8_t control = decode.read_byte();
            boost::asio::async_read(socket, input_buffer,
                boost::asio::transfer_exactly(packet_size), yield);
            fostlib::log::debug(fostlib::c_rask_proto)
                ("", "Got packet")
                ("connection", id)
                ("control", control)
                ("size", packet_size);
        } catch ( ... ) {
            socket.close();
            fostlib::log::error(c_rask_proto)
                ("", "Socket error")
                ("connection", id);
        }
    }
    fostlib::log::error(fostlib::c_rask_proto)
        ("", "Connection closed")
        ("connection", id);
}

