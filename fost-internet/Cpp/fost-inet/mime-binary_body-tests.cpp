/*
    Copyright 2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/mime.hpp>


using namespace fostlib;


FSL_TEST_SUITE( mime__binary_body );


FSL_TEST_FUNCTION( constructor ) {
    // 256 bytes of the value 123
    std::vector< unsigned char > data(256, 123);

    fostlib::binary_body body(data);
    FSL_CHECK(body.data() == data);
}
