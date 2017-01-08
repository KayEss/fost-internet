/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <f5/threading/boost-asio.hpp>

#include <fost/rask/rask-proto.hpp>

#include <boost/asio/streambuf.hpp>


namespace fostlib {


    class rask_protocol;


    /// Base class for the connection
    class rask_connection_base {
    protected:
        rask_connection_base();

    public:
        /// The connection ID used in log messages
        const int64_t id;

        /// An input buffer
        boost::asio::streambuf input_buffer;
    };


    /// A connection over a specific transport  of the Rask meta-protocol. The
    /// connection can be started from here to a transport end point that
    /// is waiting for connections, or it can be started from the transport
    /// end point that is listening for new connections,
    /// [see `class rask_server`](#class-rask_server).
    template<typename Transport>
    class rask_connection : public rask_connection_base {
    public:
        /// Default construct the connection
        rask_connection() {}

        /// TODO: Connect to a remote host
    };


    //// class-rask-server
    template<typename Transport>
    class rask_server {
        /// TODO: Listen for incoming connections
    };


    /// Start a client connection and keep it open
    template<typename Transport>
    class rask_connect {
    };


}

