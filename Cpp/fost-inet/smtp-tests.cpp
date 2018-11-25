/*
    Copyright 2011-2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/mime.hpp>


using namespace fostlib;


FSL_TEST_SUITE(smtp);


FSL_TEST_FUNCTION(email_address_without_name) {
    fostlib::json e1("test@example.com");
    fostlib::email_address a1 = fostlib::coerce<fostlib::email_address>(e1);
    FSL_CHECK(not a1.name());
    FSL_CHECK_EQ(a1.email(), "test@example.com");
}

FSL_TEST_FUNCTION(email_address_with_name) {
    fostlib::json e1("Test account<test@example.com>");
    fostlib::email_address a1 = fostlib::coerce<fostlib::email_address>(e1);
    FSL_CHECK(a1.name());
    FSL_CHECK_EQ(a1.name().value(), "Test account");
    FSL_CHECK_EQ(a1.email(), "test@example.com");

    fostlib::json e2("Test account <test@example.com>");
    fostlib::email_address a2 = fostlib::coerce<fostlib::email_address>(e2);
    FSL_CHECK(a2.name());
    FSL_CHECK_EQ(a2.name().value(), "Test account");
    FSL_CHECK_EQ(a2.email(), "test@example.com");
}
