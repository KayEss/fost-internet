/*
    Copyright 2011, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/mime.hpp>


fostlib::utf8_string fostlib::coercer<
    fostlib::utf8_string, fostlib::mime
>::coerce( const fostlib::mime &input ) {
    fostlib::utf8_string string;
    for ( fostlib::mime::const_iterator i(input.begin()); i != input.end(); ++i ) {
        fostlib::utf8_string data(fostlib::coerce<fostlib::utf8_string>(*i));
        if ( data.underlying().length() < bytes(*i) )
            throw fostlib::exceptions::not_implemented("Not all data converted");
        string += data;
    }
    return string;
}
