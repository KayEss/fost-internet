/*
    Copyright 2010, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/mime.hpp>


using namespace fostlib;


FSL_TEST_SUITE( smtp );


FSL_TEST_FUNCTION( email_address ) {
    fostlib::json e1("test@example.com");
    fostlib::email_address a1 = fostlib::coerce<fostlib::email_address>(e1);
    FSL_CHECK(a1.name().isnull());
    FSL_CHECK_EQ(a1.email(), "test@example.com");
}
