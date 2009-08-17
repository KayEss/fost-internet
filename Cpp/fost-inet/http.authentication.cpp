/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/http.authentication.fost.hpp>


void fostlib::http::fost_authentication(
    const fostlib::ascii_string &api_key, const fostlib::ascii_string &secret,
    fostlib::http::user_agent::request &request
) {
    // request.headers().add( L"X-FOST-Timestamp", now );
}

