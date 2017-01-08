/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/rask/decoder.hpp>

#include <fost/pointers>
#include <fost/unicode>

#include <boost/asio/streambuf.hpp>
#include <boost/endian/conversion.hpp>


namespace rask {

    /// The type for the control byte. Typically this should be an
    /// enumeration.
    typedef uint8_t control_byte;


    class in_packet_base : boost::noncopyable {
    };


    template<typename Store>
    class in_packet : public in_packet_base {
    };


    /// A packet that is to be sent over a connection
    class out_packet final : boost::noncopyable {
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
        std::size_t size() const {
            return buffer->size();
        }

        /// Add the specified bytes to the buffer
        template<typename B>
        void bytes(fostlib::array_view<B> av) {
            static_assert(sizeof(B), "Must add an array of bytes");
            buffer->sputn(av.data(), av.size());
        }

        /// Add a single byte to the buffer
        template<typename B>
        void byte(B b) {
            static_assert(sizeof(B), "Must add an array of bytes");
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
                reinterpret_cast<const char *>(b.second) -
                reinterpret_cast<const char *>(b.first));
        }

        /// Send the data over the wire. A concrete implementation of
        /// this is needed depending on the transport. See [tcp.hpp] for
        /// an example.
        template<typename Socket, typename Yield>
        void operator () (Socket &s, Yield &y) const;
    };


}


/// Insert an integer in network byte order
template<typename I,
    typename = std::enable_if_t<std::is_integral<I>::value>> inline
rask::out_packet &operator << (rask::out_packet &o, I i) {
    if ( sizeof(i) > 1 ) { // TODO: Should be constexpr if
        auto v = boost::endian::native_to_big(i);
        o.bytes(fostlib::array_view<char>(reinterpret_cast<char*>(&v), sizeof(v)));
    } else {
        o.byte(i);
    }
    return o;
}


/// Insert a UTF8 string
inline
rask::out_packet &operator << (rask::out_packet &o, fostlib::utf::u8_view str) {
    o.size_sequence(str.bytes());
    o.bytes(fostlib::array_view<char>(str.data(), str.bytes()));
    return o;
}

