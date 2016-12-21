/*
    Copyright 2010-2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/parse/host.hpp>
#include <fost/exception/parse_error.hpp>


using namespace fostlib;


FSL_TEST_SUITE( host );


FSL_TEST_FUNCTION( coerce_string ) {
    fostlib::host h1 = fostlib::coerce< fostlib::host >(
        fostlib::string("localhost:80"));
    FSL_CHECK_EQ( h1.name(), "localhost" );
    FSL_CHECK(h1.service());
    FSL_CHECK_EQ( h1.service().value(), "80" );

    FSL_CHECK_EQ( fostlib::coerce< fostlib::string >(h1), "localhost:80" );
}


FSL_TEST_FUNCTION( normalise_case ) {
    FSL_CHECK_EQ(fostlib::host("LOCALHOST").name(), "localhost");
    FSL_CHECK_EQ(fostlib::host("LOCALHOST", fostlib::string("http")).name(), "localhost");
    FSL_CHECK_EQ(fostlib::host("LOCALHOST", 80).name(), "localhost");
}
