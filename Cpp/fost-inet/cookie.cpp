/*
    Copyright 2016 Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/headers.hpp>
#include <fost/url.hpp>


namespace {
    const auto unescape = [](const auto &s) {
            const fostlib::url::filepath_string fs(s.c_str());
            return fostlib::coerce<fostlib::string>(fs);
        };
}


void fostlib::parse_cookies(fostlib::headers_base::content &cookie) {
    auto cookies = fostlib::split(cookie.value(), "; ");
    for ( const auto &cv : cookies ) {
        const auto parts = partition(cv, "=");
        cookie.subvalue(unescape(parts.first),
            unescape(parts.second.value_or(fostlib::string())));
    }
    cookie.value(fostlib::string());
}

