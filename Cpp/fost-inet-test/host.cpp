/*
    Copyright 2008-2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/parse/host.hpp>


using namespace fostlib;


FSL_TEST_SUITE( internet_host );


FSL_TEST_FUNCTION( constructors ) {
    FSL_CHECK_EQ(host().name(), "");
    FSL_CHECK(not host().service());
    FSL_CHECK_EQ(host("localhost").name(), "localhost");
    FSL_CHECK_EQ(host(127, 0, 0, 1).name(), "127.0.0.1");
    FSL_CHECK_EQ(host(127 << 24).name(), "127.0.0.0");
}


FSL_TEST_FUNCTION( parse ) {
    auto check = [](std::string name) {
            parser_lock lock;
            host h;
            FSL_CHECK(host_p(lock, name.begin(), name.end(), h));
            FSL_CHECK_EQ(fostlib::coerce<ascii_string>(h), fostlib::ascii_string(name));
        };
    check("localhost");
    check("www.felspar.com");
    check("127.0.0.1");

    check("localhost:80");
}


FSL_TEST_FUNCTION( resolution ) {
    FSL_CHECK_NOTHROW(host("localhost").address());
}
