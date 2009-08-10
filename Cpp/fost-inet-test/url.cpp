/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/parse/url.hpp>
#include <fost/exception/parse_error.hpp>


using namespace fostlib;


FSL_TEST_SUITE( internet_url );


FSL_TEST_FUNCTION( filepath_string ) {
    FSL_CHECK_NOTHROW( url::filepath_string a( "a/bc.html" ) );
    FSL_CHECK_EXCEPTION( url::filepath_string a( "a/b(c).html" ), fostlib::exceptions::parse_error );
    FSL_CHECK_EXCEPTION( url::filepath_string a( "a/b%" ), fostlib::exceptions::parse_error );
    FSL_CHECK_EXCEPTION( url::filepath_string a( "a/b%%" ), fostlib::exceptions::parse_error );
    FSL_CHECK_EXCEPTION( url::filepath_string a( "a/b%2" ), fostlib::exceptions::parse_error );
    FSL_CHECK_NOTHROW( url::filepath_string a( "a/bc%2B.html" ) );

    FSL_CHECK_EQ( coerce< url::filepath_string >( string( L"abc" ) ), url::filepath_string( "abc" ) );
    FSL_CHECK_EQ( coerce< url::filepath_string >( string( L"a/bc.html" ) ), url::filepath_string( "a/bc.html" ) );
    FSL_CHECK_EQ( coerce< url::filepath_string >( string( L"a/b(c).html" ) ), url::filepath_string( "a/b%28c%29.html" ) );
}


FSL_TEST_FUNCTION( query_string ) {
    url::query_string q1, q2;
    FSL_CHECK_EQ( q1.as_string().value( ascii_string() ), q2.as_string().value( ascii_string() ) );
    q1 = q2;
    q1.append( L"key", null );
    FSL_CHECK_EQ( q1.as_string().value(), ascii_string( "key=" ) );
    q1.append( L"key", null );
    FSL_CHECK_EQ( q1.as_string().value(), ascii_string( "key=&key=" ) );
    q2 = q1;
    FSL_CHECK_EQ( q2.as_string().value(), ascii_string( "key=&key=" ) );
    q1.append( L"key", L"(.)" );
    FSL_CHECK_EQ( q1.as_string().value(), ascii_string( "key=&key=&key=%28.%29" ) );
    FSL_CHECK_EQ( q2.as_string().value(), ascii_string( "key=&key=" ) );
    q2.append( L"key", L"\x2014" );
    FSL_CHECK_EQ( q1.as_string().value(), ascii_string( "key=&key=&key=%28.%29" ) );
    FSL_CHECK_EQ( q2.as_string().value(), ascii_string( "key=&key=&key=%E2%80%94" ) );
}


FSL_TEST_FUNCTION( url ) {
    FSL_CHECK_EQ( url().as_string(), ascii_string( "http://localhost/" ) );
}


#define QS_PARSE( str ) \
    FSL_CHECK( boost::spirit::parse( (str), query_string_p[ phoenix::var( qs ) = phoenix::arg1 ] ).full ); \
    FSL_CHECK_EQ( qs.as_string().value(), ascii_string(coerce< utf8string >(string(str))) );
FSL_TEST_FUNCTION( query_string_parser ) {
    url::query_string qs;
    FSL_CHECK( boost::spirit::parse( L"", query_string_p[ phoenix::var( qs ) = phoenix::arg1 ] ).full );
    FSL_CHECK( qs.as_string().isnull() );
    QS_PARSE( L"key=value&key=value" );
    QS_PARSE( L"key=value" );
    QS_PARSE( L"key=" );
    QS_PARSE( L"key=&key=" );
    QS_PARSE( L"key=value&key=" );
}


#define URL_PARSE_HOSTPART( str, u_ ) \
    FSL_CHECK( boost::spirit::parse( str, url_hostpart_p[ phoenix::var( u ) = phoenix::arg1 ] ).full ); \
    FSL_CHECK_EQ( u.as_string(), u_.as_string() );
FSL_TEST_FUNCTION( url_parser_hostpart ) {
    url u;
    URL_PARSE_HOSTPART( L"http://localhost/", url() );
    URL_PARSE_HOSTPART( L"http://127.0.0.1/", url( host( 127, 0, 0, 1 ) ) );
    URL_PARSE_HOSTPART( L"http://10.0.2.2/", url( host( 10, 0, 2, 2 ) ) );
    URL_PARSE_HOSTPART( L"http://www.felspar.com/", url( host( L"www.felspar.com" ) ) );
    FSL_CHECK( !boost::spirit::parse( L"http://www..felspar.com/", url_hostpart_p ).full );
    FSL_CHECK( !boost::spirit::parse( L"http://www./", url_hostpart_p ).full );
    FSL_CHECK( !boost::spirit::parse( L"http://.www/", url_hostpart_p ).full );
    URL_PARSE_HOSTPART( L"http://123.45/", url( host( L"123.45" ) ) );
    URL_PARSE_HOSTPART( L"http://12345/", url( host( 12345 ) ) );
}
#define URL_PARSE_FILESPEC( str, s_ ) \
    FSL_CHECK( boost::spirit::parse( str, url_filespec_p[ phoenix::var( s ) = phoenix::arg1 ] ).full ); \
    FSL_CHECK_EQ( s, ascii_string( s_ ) )
FSL_TEST_FUNCTION( url_parser_filespec ) {
    ascii_string s;
    URL_PARSE_FILESPEC( L"/", "/" );
    URL_PARSE_FILESPEC( L"/file.html", "/file.html" );
    URL_PARSE_FILESPEC( L"/Site:/file.html", "/Site:/file.html" );
}

FSL_TEST_FUNCTION( path_spec ) {
    url u( L"http://localhost/" );
    u.pathspec( url::filepath_string( "/file-name" ) );
    FSL_CHECK_EQ( u.as_string(), ascii_string( "http://localhost/file-name" ) );
}


FSL_TEST_FUNCTION( parse ) {
    FSL_CHECK_NOTHROW(
        url a( L"http://localhost/" );
        FSL_CHECK_EQ( a.server().name(), L"localhost" );
        FSL_CHECK( a.user().isnull() );
    )

    FSL_CHECK_EQ( url( "http://localhost/file-path.html" ).pathspec(), url::filepath_string( "/file-path.html" ) );

    FSL_CHECK_EXCEPTION( url( "http://localhost/file path.html" ), fostlib::exceptions::parse_error& );
    FSL_CHECK_EXCEPTION( url( "http://localhost/file\\path.html" ), fostlib::exceptions::parse_error& );
}

FSL_TEST_FUNCTION( coercion ) {
    FSL_CHECK_EQ( coerce< string >( url( "http://localhost/file-path.html" ) ), L"http://localhost/file-path.html" );
}

/*
FSL_TEST_FUNCTION( parse_port ) {
    url  a( L"http://localhost:8000/" );
    FSL_CHECK_EQ( a.server().name(), L"localhost" );
    FSL_CHECK( a.user().isnull() );
    FSL_CHECK_EQ( a.server().service(), L"8000" );
}

FSL_TEST_FUNCTION( parse_credentials ) {
    url  b( L"http://user:pass@localhost/" );
    FSL_CHECK_EQ( b.server().name(), L"localhost" );
    FSL_CHECK( !b.user().isnull() );
    FSL_CHECK_EQ( b.user().value(), L"user" );
    FSL_CHECK_EQ( b.password().value(), L"pass" );
}

FSL_TEST_FUNCTION( parse_credentials_port ) {
    url  c( L"http://user:pass@localhost:8000/" );
    FSL_CHECK_EQ( c.server().name(), L"localhost" );
    FSL_CHECK( !c.user().isnull() );
    FSL_CHECK_EQ( c.user().value(), L"user" );
    FSL_CHECK_EQ( c.password().value(), L"pass" );
    FSL_CHECK_EQ( c.server().service(), L"8000" );
}
*/