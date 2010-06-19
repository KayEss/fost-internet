/*
    Copyright 2010, Felspar Co Ltd. http://fost.3.felspar.com/
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
        fostlib::string("localhost:80")
    );
    FSL_CHECK_EQ( h1.name(), "localhost" );
    FSL_CHECK( !h1.service().isnull() );
    FSL_CHECK_EQ( h1.service().value(), "80" );
}
