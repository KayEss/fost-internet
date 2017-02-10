/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <f5/threading/boost-asio.hpp>

#include <fost/rask/rask-proto.hpp>

#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>


namespace fostlib {


    class rask_protocol;


    /// Base class for the connection
    class rask_connection_base :
        public std::enable_shared_from_this<rask_connection_base>
    {
    protected:
        rask_connection_base();

    public:
        /// The connection ID used in log messages
        const int64_t id;

    };


    /// A connection over a specific transport  of the Rask meta-protocol. The
    /// connection can be started from here to a transport end point that
    /// is waiting for connections, or it can be started from the transport
    /// end point that is listening for new connections,
    /// [see `class rask_server`](#class-rask_server).
    template<typename Transport>
    class rask_connection : public rask_connection_base {
    protected:
        /// Which side is this peer
        using peering = enum { server_side, client_side };
        const peering peer;

        /// Construct a connection
        rask_connection(peering p)
        : peer(p) {
        }

        /// Return the IO service that needs to be used to service
        /// the underlying connection
        virtual boost::asio::io_service &get_io_service() = 0;

    public:
        /// Start up the data sending and receiving processes
        virtual void process() {
            auto self = shared_from_this();
            boost::asio::spawn(get_io_service(), [self, this](auto yield) {
                this->process_inbound(yield);
            });
            boost::asio::spawn(get_io_service(), [self, this](auto yield) {
                this->process_outbound(yield);
            });
        }

    protected:
        /// The inbound message stream
        virtual void process_inbound(boost::asio::yield_context &) = 0;
        /// Dispatch the packet
        virtual void dispatch(uint8_t control, std::size_t bytes, boost::asio::streambuf &) = 0;

        /// The outbound message stream
        virtual void process_outbound(boost::asio::yield_context &) = 0;
    };


    /// Start a client connection and keep it open
    template<typename Transport>
    class rask_connect {
    };


}

