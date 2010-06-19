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


FSL_TEST_FUNCTION( field_setting ) {
    fostlib::mime::mime_headers headers;
    FSL_CHECK( headers.end() == headers.begin() );
    headers.set("H1", "a");
    FSL_CHECK( headers.end() != headers.begin() );
    FSL_CHECK( ++headers.begin() == headers.end() );
    headers.set("H1", "b"); // Replaces the content
    FSL_CHECK( headers.end() != headers.begin() );
    FSL_CHECK( ++headers.begin() == headers.end() );
    FSL_CHECK_EQ(headers["H1"].value(), "b");

    // Check sub values can be set properly
    headers.set_subvalue("H1", "sub", "v1");
    FSL_CHECK( headers.end() != headers.begin() );
    FSL_CHECK( ++headers.begin() == headers.end() );
    FSL_CHECK_EQ(headers["H1"].subvalue("sub").value(), "v1");

    headers.set("H1", "c"); // Replaces the content
    FSL_CHECK( headers.end() != headers.begin() );
    FSL_CHECK( ++headers.begin() == headers.end() );
    FSL_CHECK(headers["H1"].subvalue("sub").isnull());

    headers.add("H1", "d"); // Adds a second H1 header
    FSL_CHECK( headers.end() != headers.begin() );
    FSL_CHECK( ++headers.begin() != headers.end() );
    FSL_CHECK( ++ ++headers.begin() == headers.end() );
}

