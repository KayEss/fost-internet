/*
    Copyright 2010, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/internet>


using namespace fostlib;


FSL_TEST_SUITE( connection );


FSL_TEST_FUNCTION( connect ) {
    FSL_CHECK_EXCEPTION(
        network_connection cnx(host("localhost"), 64544),
        exceptions::connect_failure&);
    FSL_CHECK_EXCEPTION(
        network_connection cnx(host("1.1.1.1"), 64544),
        exceptions::connect_failure&);
}
