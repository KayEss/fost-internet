/**
    Copyright 2017-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/hod/decoder.hpp>

#include <fost/counter>
#include <fost/pointers>
#include <fost/unicode>

#include <boost/asio/streambuf.hpp>
#include <boost/endian/conversion.hpp>


namespace fostlib {
    namespace hod {


        /// The type for the control byte. Typically this should be an
        /// enumeration.
        typedef uint8_t control_byte;


        /// A packet that is to be sent over a connection
        class out_packet final : boost::noncopyable {
            static fostlib::performance p_created;
            static fostlib::performance p_sent;

          private:
            /// Output buffers
            std::unique_ptr<boost::asio::streambuf> buffer;
            /// The control block value
            control_byte control;

            /// Write a size control sequence to the specified buffer
            static void size_sequence(std::size_t, boost::asio::streambuf &);

          public:
            /// Build a new outbound packet with the specified control byte.
            out_packet(control_byte);

            /// Make movable
            out_packet(out_packet &&);

            /// Return the current size of the packet (minus headers)
            std::size_t size() const { return buffer->size(); }

            /// Add the specified bytes to the buffer
            template<typename B>
            void bytes(fostlib::array_view<B> av) {
                static_assert(sizeof(B) == 1, "Must add an array of bytes");
                buffer->sputn(
                        reinterpret_cast<const char *>(av.data()), av.size());
            }

            /// Add a single byte to the buffer
            template<typename B>
            void byte(B b) {
                static_assert(sizeof(B) == 1, "Must add a single byte");
                buffer->sputc(b);
            }

            /// Add a size control sequence
            out_packet &size_sequence(std::size_t s) {
                size_sequence(s, *buffer);
                return *this;
            }

            /// Add a size control sequence for a memory block
            out_packet &size_sequence(const fostlib::const_memory_block b) {
                return size_sequence(
                        reinterpret_cast<const char *>(b.second)
                        - reinterpret_cast<const char *>(b.first));
            }

            /// Send the data over the wire. A concrete implementation of
            /// this is needed depending on the transport. See [tcp.hpp] for
            /// an example.
            template<typename Socket, typename Yield>
            void operator()(Socket &s, Yield y) const;
        };


    }
}
