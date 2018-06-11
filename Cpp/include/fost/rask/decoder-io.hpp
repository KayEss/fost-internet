/**
    Copyright 2017-2018, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once


#include <fost/rask/decoder.hpp>

#include <type_traits>


namespace fostlib { namespace hod {


    namespace detail {
        /// Throw an exception if there aren't enough bytes in the sequence
        template<typename P> inline
        void check_bytes(decoder<P> &d, std::size_t bytes) {
            if ( d.size() < bytes ) {
                fostlib::exceptions::unexpected_eof e(
                    "Tried to read more than the number of available bytes");
                fostlib::insert(e.data(), "remaining", d.size());
                fostlib::insert(e.data(), "wanted", bytes);
                throw e;
            }
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
        I operator () (decoder<P> &d) {
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
    inline out_packet &operator << (out_packet &o, f5::u8view str) {
        o.size_sequence(str.bytes());
        o.bytes(str.memory());
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


}}

