/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/rask/packet.hpp>


rask::out_packet::out_packet(control_byte c)
: buffer(new boost::asio::streambuf), control(c) {
}


rask::out_packet::out_packet(out_packet &&o)
: buffer(std::move(o.buffer)), control(o.control) {
}

