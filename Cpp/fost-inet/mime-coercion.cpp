/**
    Copyright 2011-2019, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include "fost-inet.hpp"
#include <fost/mime.hpp>
#include <fost/insert>


fostlib::utf8_string
        fostlib::coercer<fostlib::utf8_string, fostlib::mime>::coerce(
                const fostlib::mime &input) {
    fostlib::utf8_string string;
    for (fostlib::mime::const_iterator i(input.begin()); i != input.end();
         ++i) {
        fostlib::utf8_string data(fostlib::coerce<fostlib::utf8_string>(*i));
        if (data.underlying().bytes() < bytes(*i)) {
            fostlib::exceptions::not_implemented err{__PRETTY_FUNCTION__,
                                                     "Not all data converted"};
            fostlib::insert(
                    err.data(), "data", "bytes", data.underlying().bytes());
            fostlib::insert(err.data(), "bytes", bytes(*i));
            throw err;
        }
        string += data;
    }
    return string;
}
