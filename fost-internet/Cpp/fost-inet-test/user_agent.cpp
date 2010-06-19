/*
    Copyright 2009-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/http>


using namespace fostlib;


FSL_TEST_SUITE( user_agent );


FSL_TEST_FUNCTION( request ) {
    FSL_CHECK_NOTHROW(http::user_agent::request r("HEAD", url()));
}


FSL_TEST_FUNCTION( user_agent ) {
    http::user_agent ua(url("http://www.google.com/"));
    http::user_agent::request r("HEAD", ua.base());
    ua(r);
}


FSL_TEST_FUNCTION( mime_request ) {
    boost::shared_ptr< mime > request_body( new empty_mime );
    http::user_agent ua(url("http://www.google.com/"));
    http::user_agent::request r("GET", ua.base(), request_body);
    FSL_CHECK_NOTHROW(ua(r));
}

