/*
    Copyright 2008-2010, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/parse/host.hpp>


using namespace fostlib;


FSL_TEST_SUITE( internet_host );


FSL_TEST_FUNCTION( constructors ) {
    FSL_CHECK_EQ( host().name(), L"" );
    FSL_CHECK( host().service().isnull() );
    FSL_CHECK_EQ( host( L"localhost" ).name(), L"localhost" );
    FSL_CHECK_EQ( host( 127, 0, 0, 1 ).name(), L"127.0.0.1" );
    FSL_CHECK_EQ( host( 127 << 24 ).name(), L"127.0.0.0" );
}


#define HOST_PARSE( s ) \
    FSL_CHECK( fostlib::parse( lock, (s), host_p[ phoenix::var(h) = phoenix::arg1 ] ).full ); \
    FSL_CHECK_EQ( fostlib::coerce< ascii_string >( h ), fostlib::ascii_string(s) );
FSL_TEST_FUNCTION( parse ) {
    fostlib::parser_lock lock;
    host_parser host_p;
    host h;
    HOST_PARSE("localhost");
    HOST_PARSE("www.felspar.com");
    HOST_PARSE("127.0.0.1");

    HOST_PARSE("localhost:80");
}


FSL_TEST_FUNCTION( resolution ) {
    FSL_CHECK_NOTHROW( host("localhost").address() );
}
