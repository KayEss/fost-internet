/*
    Copyright 2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/mime.hpp>


using namespace fostlib;


FSL_TEST_SUITE( headers );


FSL_TEST_FUNCTION( ostream ) {
    // Use a std::stringstream here because all of these MIME types need to be
    // printable on a narrow character stream as that is what all network connections
    // are.
    std::stringstream ss;
    fostlib::headers_base::content c;
    ss << c;
}
