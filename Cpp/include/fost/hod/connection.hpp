/**
    Copyright 2017-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <f5/threading/reactor.hpp>

#include <fost/hod/hod.hpp>
#include <fost/hod/protocol.hpp>

#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>

#include <deque>


namespace fostlib {
    namespace hod {


        /// Base class for the connection
        class connection_base {
          protected:
            connection_base();

            /// The version that these two peers can support for sending
            /// of data
            std::atomic<uint8_t> peer_version;

          public:
            /// The connection ID used in log messages
            const int64_t id;

            /// Return the current version number for the connection
            uint8_t version() { return peer_version.load(); }
            /// Set the negotiated version number and return the old
            /// version number
            template<typename D>
            uint8_t version(const protocol<D> &proto, uint8_t peer) {
                auto cur = peer_version.load();
                peer_version.store(std::min(proto.max_version(), peer));
                return cur;
            }
        };


        /// A connection over a specific transport  of the Rask meta-protocol.
        /// The connection can be started from here to a transport end point
        /// that is waiting for connections, or it can be started from the
        /// transport end point that is listening for new connections.
        template<typename Transport>
        class connection : public connection_base {
          public:
            /// Which side is this peer
            using peering = enum { server_side, client_side };
            const peering peer;

          protected:
            /// Construct a connection
            connection(peering p) : peer(p) {}

            /// Return the IO service that needs to be used to service
            /// the underlying connection
            virtual boost::asio::io_service &get_io_service() = 0;

          public:
            /// Start up the data sending and receiving processes
            template<typename A>
            void process(A capture) {
                boost::asio::spawn(
                        get_io_service(), [capture, this](auto yield) {
                            this->process_inbound(yield);
                        });
                boost::asio::spawn(
                        get_io_service(), [capture, this](auto yield) {
                            this->process_outbound(yield);
                        });
                established();
            }

          protected:
            /// The inbound message stream
            virtual void process_inbound(boost::asio::yield_context) = 0;
            /// The outbound message stream
            virtual void process_outbound(boost::asio::yield_context) = 0;
            /// The connection is establshed and ready to send and receive
            /// data
            virtual void established() = 0;
        };


    }
}
