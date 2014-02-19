/*
    Copyright 2008-2012, Felspar Co Ltd. http://support.felspar.com/
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
    FSL_CHECK_EXCEPTION( url::filepath_string a( "a/b c.html" ),
        fostlib::exceptions::parse_error );
    FSL_CHECK_EXCEPTION( url::filepath_string a( "a/b%" ), fostlib::exceptions::parse_error );
    FSL_CHECK_EXCEPTION( url::filepath_string a( "a/b%%" ), fostlib::exceptions::parse_error );
    FSL_CHECK_EXCEPTION( url::filepath_string a( "a/b%2" ), fostlib::exceptions::parse_error );
    FSL_CHECK_NOTHROW( url::filepath_string a( "a/bc%2B.html" ) );

    FSL_CHECK_EQ( coerce< url::filepath_string >( string( L"abc" ) ),
        url::filepath_string( "abc" ) );
    FSL_CHECK_EQ( coerce< url::filepath_string >( string( L"a/bc.html" ) ),
        url::filepath_string( "a/bc.html" ) );
    FSL_CHECK_EQ( coerce< url::filepath_string >( string( L"a/b(c).html" ) ),
        url::filepath_string( "a/b%28c%29.html" ) );

    FSL_CHECK_EQ( coerce< boost::filesystem::wpath >( url::filepath_string("a") ), L"a");
    FSL_CHECK_EQ( coerce< boost::filesystem::wpath >( url::filepath_string("%2B") ), L"+");
}


FSL_TEST_FUNCTION( query_string ) {
    url::query_string q1, q2;
    FSL_CHECK( q1.as_string().isnull() );
    FSL_CHECK( q2.as_string().isnull() );
    FSL_CHECK_EQ(
        q1.as_string().value( ascii_printable_string() ),
        q2.as_string().value( ascii_printable_string() )
    );
    q1 = q2;
    q1.append( L"key", null );
    FSL_CHECK_EQ( q1.as_string().value(), ascii_printable_string( "key=" ) );
    q1.append( L"key", null );
    FSL_CHECK_EQ( q1.as_string().value(), ascii_printable_string( "key=&key=" ) );
    q2 = q1;
    FSL_CHECK_EQ( q2.as_string().value(), ascii_printable_string( "key=&key=" ) );
    q1.append( L"key", L"(.)" );
    FSL_CHECK_EQ(
        q1.as_string().value(),
        ascii_printable_string( "key=&key=&key=%28.%29" )
    );
    FSL_CHECK_EQ(
        q2.as_string().value(),
        ascii_printable_string( "key=&key=" )
    );
    q2.append( L"key", L"\x2014" );
    FSL_CHECK_EQ(
        q1.as_string().value(),
        ascii_printable_string( "key=&key=&key=%28.%29" )
    );
    FSL_CHECK_EQ(
        q2.as_string().value(),
        ascii_printable_string( "key=&key=&key=%E2%80%94" )
    );
}


FSL_TEST_FUNCTION( url ) {
    FSL_CHECK_EQ( url().port(), 80 );
    FSL_CHECK_EQ( url().as_string(), ascii_printable_string( "http://localhost/" ) );
    FSL_CHECK_EQ( url(url("https://localhost/"), "/path").as_string(),
        ascii_printable_string("https://localhost/path") );
}


#define QS_PARSE( str ) \
    FSL_CHECK( fostlib::parse( lock, (str), \
            query_string_p[ phoenix::var( qs ) = phoenix::arg1 ] ).full ); \
    FSL_CHECK_EQ( qs.as_string().value(), \
            coerce< ascii_printable_string >(string(str)) );
FSL_TEST_FUNCTION( query_string_parser ) {
    fostlib::parser_lock lock;
    query_string_parser query_string_p;

    url::query_string qs;
/*    FSL_CHECK( fostlib::parse( L"",
            query_string_p[ phoenix::var( qs ) = phoenix::arg1 ] ).full );
    FSL_CHECK( qs.as_string().isnull() );*/
    QS_PARSE( L"key=value&key=value" );
    QS_PARSE( L"key=value" );
    QS_PARSE( L"key=" );
    QS_PARSE( L"__=" );
    QS_PARSE( L"key=&key=" );
    QS_PARSE( L"key=value&key=" );
    QS_PARSE( L"next=/path/" );
    QS_PARSE( L"key1&key2" );
    QS_PARSE( L"key2&key1" ); // Ensure order is preserved
    QS_PARSE( L"key1=value1&key2=value=2&key3=value3" );
    QS_PARSE( L"key=%26%2312296%3B" );
}


FSL_TEST_FUNCTION( url_parser_protocol ) {
    FSL_CHECK_EQ(url("http://localhost/").protocol(), ascii_printable_string("http"));
    FSL_CHECK_EQ(url("http://localhost/").port(), 80);
    FSL_CHECK_EQ(url("https://localhost/").protocol(), ascii_printable_string("https"));
    FSL_CHECK_EQ(url("https://localhost/").port(), 443);
}


#define URL_PARSE_HOSTPART( str, u_ ) \
    FSL_CHECK( fostlib::parse( lock, str, \
        url_hostpart_p[ phoenix::var( u ) = phoenix::arg1 ] ).full ); \
    FSL_CHECK_EQ( u.as_string(), u_.as_string() );
FSL_TEST_FUNCTION( url_parser_hostpart ) {
    fostlib::parser_lock lock;
    url_hostpart_parser url_hostpart_p;

    url u;
    URL_PARSE_HOSTPART( L"http://localhost", url() );
    URL_PARSE_HOSTPART( L"http://127.0.0.1", url( host( 127, 0, 0, 1 ) ) );
    URL_PARSE_HOSTPART( L"http://10.0.2.2", url( host( 10, 0, 2, 2 ) ) );
    URL_PARSE_HOSTPART( L"http://www.felspar.com", url( host( L"www.felspar.com" ) ) );
    URL_PARSE_HOSTPART( L"http://urquell-fn.appspot.com", url( host( L"urquell-fn.appspot.com" ) ) );
    FSL_CHECK( !fostlib::parse( lock, L"http://www..felspar.com/", url_hostpart_p ).full );
    FSL_CHECK( !fostlib::parse( lock, L"http://www./", url_hostpart_p ).full );
    FSL_CHECK( !fostlib::parse( lock, L"http://.www/", url_hostpart_p ).full );
    URL_PARSE_HOSTPART( L"http://123.45", url( host( L"123.45" ) ) );
    URL_PARSE_HOSTPART( L"http://12345", url( host( 12345 ) ) );
    URL_PARSE_HOSTPART( L"http://localhost:80", url( host( L"localhost", L"80" ) ) );
    URL_PARSE_HOSTPART( L"http://localhost:8080", url( host( L"localhost", L"8080" ) ) );
}
#define URL_PARSE_FILESPEC( str, s_ ) \
    FSL_CHECK( fostlib::parse( lock, str, url_filespec_p[ phoenix::var( s ) = phoenix::arg1 ] ).full ); \
    FSL_CHECK_EQ( s, ascii_printable_string( s_ ) )
FSL_TEST_FUNCTION( url_parser_filespec ) {
    fostlib::parser_lock lock;
    url_filespec_parser url_filespec_p;

    ascii_printable_string s;
    URL_PARSE_FILESPEC( "/", "/" );
    URL_PARSE_FILESPEC( "/*/", "/*/" );
    URL_PARSE_FILESPEC( "/file.html", "/file.html" );
    URL_PARSE_FILESPEC( "/Site:/file.html", "/Site:/file.html" );
    URL_PARSE_FILESPEC( "/Site:/(file).html", "/Site:/(file).html" );
    URL_PARSE_FILESPEC( "/Type/List:/Article%20(FSLib::::Content::::Article)",
        "/Type/List:/Article%20(FSLib::::Content::::Article)" );
}

FSL_TEST_FUNCTION( path_spec ) {
    url u( L"http://localhost/" );
    u.pathspec( url::filepath_string( "/file-name" ) );
    FSL_CHECK_EQ( u.as_string(), ascii_printable_string( "http://localhost/file-name" ) );

    FSL_CHECK_EQ(
        coerce< url::filepath_string >( boost::filesystem::wpath(L"test") ),
        url::filepath_string("test") );
    FSL_CHECK_EQ(coerce<string>(u.pathspec()), "/file-name");
}

#define TEST_PATH_SPEC_ENCODING( from, to ) \
    FSL_CHECK_EQ( url::filepath_string(ascii_printable_string(from), url::filepath_string::unencoded).underlying(), to )
FSL_TEST_FUNCTION( path_spec_encoding ) {
    TEST_PATH_SPEC_ENCODING("invalid@felspar.com", "invalid%40felspar.com");
    TEST_PATH_SPEC_ENCODING("/@~:.-_+", "/%40~:.-_%2B");
}


FSL_TEST_FUNCTION( parse ) {
    FSL_CHECK_NOTHROW(
        url a( L"http://localhost/" );
        FSL_CHECK_EQ( a.server().name(), L"localhost" );
        FSL_CHECK( a.user().isnull() );
    )
    FSL_CHECK_EQ( url( "http://localhost" ).server().name(), L"localhost" );
    FSL_CHECK_EQ( url( "http://localhost/file-path.html" ).pathspec(), url::filepath_string( "/file-path.html" ) );
    FSL_CHECK_EQ( url( "http://localhost:6789/file-path.html" ).server().service().value(), L"6789" );
    FSL_CHECK_EQ( url( "http://localhost:6789/file-path.html" ).port(), 6789 );

    FSL_CHECK_EXCEPTION( url( "http://localhost/file path.html" ), fostlib::exceptions::parse_error& );
    FSL_CHECK_EXCEPTION( url( "http://localhost/file\\path.html" ), fostlib::exceptions::parse_error& );

    FSL_CHECK_EQ(
        url("http://bmf.miro.felspar.net:8000/rest/email/new_subscription/123821/").pathspec(),
        url::filepath_string("/rest/email/new_subscription/123821/")
    );
    FSL_CHECK_NOTHROW(url("http://urquell-fn.appspot.com/lib/echo/*Afsk1YSP"));
    FSL_CHECK_NOTHROW(url("http://urquell-fn.appspot.com/lib/json/object/basic_data"));
    FSL_CHECK_NOTHROW(url("http://urquell-fn.appspot.com/lib/json/object/basic_data?__="));
    FSL_CHECK_NOTHROW(url("http://l.yimg.com/a/combo?"
        "arc/yui/reset_2.6.7.css&"
        "arc/yui/fonts_2.6.4.css&metro/uiplugins/generic_0.1.13.css&"
        "metro/error/error_0.1.18.css&metro/fp/fp_zindex_0.0.34.css&"
        "metro/fp/fp_0.1.101.css&metro/uiplugins/tablist_service_0.1.7.css&"
        "metro/uiplugins/iframeshim_service_0.0.5.css&"
        "metro/uiplugins/menu_service_0.1.4.css&"
        "metro/masthead/masthead_0.2.101.css&"
        "metro/navbar/navbar_0.1.119.css&"
        "metro/navbar/navbar_pageoptions_0.0.44.css&"
        "metro/sda/sda_0.1.37.css&"
        "metro/tuc/tuc_outboxlite_common_0.0.24.css&"
        "metro/tuc/tuc_outboxlite_embedded_0.0.12.css&"
        "metro/pa/pa_0.1.196.css&"
        "metro/pa/pa_detached_0.1.84.css&"
        "metro/uiplugins/tooltip_service_1.0.5.css&"
        "metro/uiplugins/tooltip_default_0.1.19.css&"
        "metro/pa/tooltip_pa_dialog_1.0.12.css&"
        "metro/uiplugins/sortable_service_0.1.9.css&"
        "metro/pa/pa_add_0.1.55.css&"
        "metro/uiplugins/tablist_news_0.0.16.css&"
        "metro2/simplenews/simplenews_0.1.19.css&"
        "metro/footer/footer_0.1.68.css&metro/footer/subfooter_0.0.11.css"));
    FSL_CHECK_NOTHROW(url("http://www.google.com/coop/cse/brand?"
        "form=cse-search-box%26%2312296%3B=th"));
}

#define TEST_COERCION(u) \
    FSL_CHECK_EQ( coerce< string >( url( u ) ), u );
FSL_TEST_FUNCTION( z_coercion ) {
    TEST_COERCION( "http://localhost/file-path.html" );
    TEST_COERCION( "http://localhost/somebody@example.com" );
    TEST_COERCION( "http://localhost/somebody+else@example.com" );
    TEST_COERCION( "http://localhost/~somebody" );
    TEST_COERCION( "http://localhost:8000/~somebody" );
    TEST_COERCION( "http://bmf.miro.felspar.net/extranet/login/?next=/office/" );
    TEST_COERCION( "http://bmf.miro.felspar.net:8000/extranet/login/?next=/office/" );
}

