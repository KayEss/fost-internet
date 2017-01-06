/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/rask/rask-proto.hpp>

#include <boost/asio/streambuf.hpp>


namespace rask {

    /// The type for the control byte. Typically this should be an
    /// enumeration.
    typedef uint8_t control_byte;


    class in_packet_base : boost::noncopyable {
    };


    template<typename Iterator, typename Store>
    class in_packet : public in_packet_base {
    };


    /// A packet that is to be sent over a connection
    class out_packet : boost::noncopyable {
    protected:
        /// Output buffers
        std::unique_ptr<boost::asio::streambuf> buffer;
        /// The control block value
        control_byte control;

        out_packet(control_byte);

    public:
        out_packet(out_packet &&);
    };


}

