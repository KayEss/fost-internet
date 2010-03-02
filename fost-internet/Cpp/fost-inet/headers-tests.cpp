/*
    Copyright 2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/detail/mime.hpp>


using namespace fostlib;


FSL_TEST_SUITE( headers );


FSL_TEST_FUNCTION( ostream ) {
    fostlib::stringstream ss;
    fostlib::headers_base::content c;
    ss << c;
}
