/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/rask/decoder.hpp>

#include <type_traits>


namespace rask {


    namespace detail {
        /// Throw an exception if there aren't enough bytes in the sequence
        template<typename P> inline
        void check_bytes(decoder<P> &d, std::size_t bytes) {
        }
    }


    /// struct to be partially specialised for all the types we need to
    /// be able to send and receive.
    template<typename V, typename P, typename = void>
    struct read_pattern {};


    /// Read an instance of the specified type from the packet
    template<typename V, typename P>
    V read(decoder<P> &d) {
        return read_pattern<V, P>{}(d);
    }


    /// Insert an integer in network byte order
    template<typename I > inline
    std::enable_if_t<std::is_integral<I>::value && (sizeof(I) > 1), out_packet &>
        operator << (out_packet &o, I i)
    {
        auto v = boost::endian::native_to_big(i);
        o.bytes(fostlib::array_view<char>(reinterpret_cast<char*>(&v), sizeof(v)));
        return o;
    }
    template<typename I> inline
    std::enable_if_t<std::is_integral<I>::value && sizeof(I) == 1, out_packet &>
        operator << (out_packet &o, I i)
    {
        o.byte(i);
        return o;
    }
    /// Read an integer type
    template<typename I, typename P>
    struct read_pattern<I, P, std::enable_if_t<std::is_integral<I>::value>>
    {
        I operator () (rask::decoder<P> &d) {
            detail::check_bytes(d, sizeof(I));
            if ( sizeof(I) > 1 ) { // TODO: Should be constexpr if
                I i;
                d.read_data(reinterpret_cast<char *>(&i), sizeof(I));
                return boost::endian::big_to_native(i);
            } else {
                return d.read_byte();
            }
        }
    };


    /// Insert a UTF8 string
    inline out_packet &operator << (out_packet &o, fostlib::utf::u8_view str) {
        o.size_sequence(str.bytes());
        o.bytes(fostlib::array_view<char>(str.data(), str.bytes()));
        return o;
    }
    /// Read a string
    template<typename P>
    struct read_pattern<fostlib::utf8_string, P> {
        fostlib::utf8_string operator () (decoder<P> &d) {
            auto length = d.read_size();
            fostlib::utf8_string str;
            str.reserve(length);
            while ( length-- ) {
                str += d.read_byte();
            }
            return str;
        }
    };


}

