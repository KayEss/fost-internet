/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/http>


using namespace fostlib;


FSL_TEST_SUITE( authentication );


FSL_TEST_FUNCTION( fost_authentication ) {
    fostlib::http::user_agent::request r("GET", fostlib::url());
    fostlib::http::fost_authentication("Not a key", "Not a secret", std::set< fostlib::string >(), r);
    FSL_CHECK(r.headers().exists("Authorization"));
    FSL_CHECK(r.headers().exists("X-FOST-Timestamp"));
    FSL_CHECK_EQ(r.headers()["X-FOST-Headers"].value(), L"X-FOST-Headers");
}
