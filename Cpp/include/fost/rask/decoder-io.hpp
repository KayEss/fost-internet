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


    /// Read an integer type
    template<typename I, typename P> inline
    typename std::enable_if<std::is_integral<I>::value, I>::type read(decoder<P> &d) {
        detail::check_bytes(d, sizeof(I));
        if ( sizeof(I) ) {
            return d.read_byte();
        } else {
            I i;
            d.read_data(reinterpret_cast<char *>(&i), sizeof(I));
            boost::endian::big_to_native_inplace(i);
        }
    }


}

