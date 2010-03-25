/*
    Copyright 2008-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/mime.hpp>


using namespace fostlib;


FSL_TEST_SUITE( mime );


FSL_TEST_FUNCTION( headers ) {
    mime::mime_headers headers;
    headers.parse( L"X-First: value\r\nX-Second: value" );
    FSL_CHECK( headers.exists( L"X-First" ) );
    FSL_CHECK( headers.exists( L"X-Second" ) );
    FSL_CHECK( !headers.exists( L"X-Third" ) );

    FSL_CHECK_EQ( headers[ L"X-First" ].value(), L"value" );
    FSL_CHECK_EQ( headers[ L"X-Second" ].value(), L"value" );

    headers.set("X-First", L"Another value");
    FSL_CHECK_EQ( headers[ L"X-First" ].value(), L"Another value" );
}

FSL_TEST_FUNCTION( headers_without_values_are_legit ) {
    mime::mime_headers headers;
    // from an Exchange bounced message:
    headers.parse( L"X-MS-TNEF-Correlator: \r\n" );
    headers.set("TE");

    std::ostringstream ss;
    ss << headers;
    FSL_CHECK_EQ("TE: \r\nX-MS-TNEF-Correlator: \r\n", ss.str());
}

FSL_TEST_FUNCTION( empty_mime ) {
    empty_mime empty;
    std::stringstream ss;
    ss << empty;
    FSL_CHECK_EQ( ss.str(), "\
Content-Length: 0\r\n\
Content-Type: application/x-empty\r\n\
\r\n\
" );
}

FSL_TEST_FUNCTION( empty_mime_with_headers ) {
    empty_mime empty;
    empty.headers().set("Host", fostlib::string("localhost"));
    empty.headers().set("User-Agent", fostlib::string("Fake agent"));
    std::stringstream ss;
    ss << empty;
    FSL_CHECK_EQ( ss.str(), "\
Content-Length: 0\r\n\
Content-Type: application/x-empty\r\n\
Host: localhost\r\n\
User-Agent: Fake agent\r\n\
\r\n\
" );
}

FSL_TEST_FUNCTION(text1) {
    text_body ta(L"Test text document");
    std::stringstream ss;
    ss << ta;
    mime::mime_headers headers;
    headers.parse( coerce< string >( partition( utf8_string( ss.str() ), "\r\n\r\n" ).first ) );
    FSL_CHECK_EQ( utf8_string( ss.str() ), "\
Content-Length: 18\r\n\
Content-Transfer-Encoding: 8bit\r\n\
Content-Type: text/plain; charset=\"utf-8\"\r\n\
\r\n\
Test text document" );
}
FSL_TEST_FUNCTION(text2) {
    text_body ta(utf8_string("Test text document"));
    std::stringstream ss;
    ss << ta;
    mime::mime_headers headers;
    headers.parse( coerce< string >( partition( utf8_string( ss.str() ), "\r\n\r\n" ).first ) );
    FSL_CHECK_EQ( utf8_string( ss.str() ), "\
Content-Length: 18\r\n\
Content-Transfer-Encoding: 8bit\r\n\
Content-Type: text/plain; charset=\"utf-8\"\r\n\
\r\n\
Test text document" );
}
FSL_TEST_FUNCTION(text3) {
    utf8 b[19];
    std::strncpy(reinterpret_cast< char * >(b), "Test text document", 19);
    text_body ta(b, b+18);
    std::stringstream ss;
    ss << ta;
    mime::mime_headers headers;
    headers.parse( coerce< string >( partition( utf8_string( ss.str() ), "\r\n\r\n" ).first ) );
    FSL_CHECK_EQ( utf8_string( ss.str() ), "\
Content-Length: 18\r\n\
Content-Transfer-Encoding: 8bit\r\n\
Content-Type: text/plain; charset=\"utf-8\"\r\n\
\r\n\
Test text document" );
}


FSL_TEST_FUNCTION( mime_attachment ) {
    mime_envelope envelope;
    envelope.items().push_back( boost::shared_ptr< mime >( new text_body( L"Test text document" ) ) );
    std::stringstream ss;
    ss << envelope;
    mime::mime_headers headers;
    headers.parse( coerce< string >( partition( utf8_string( ss.str() ), "\r\n\r\n" ).first ) );
    try {
        FSL_CHECK_EQ( utf8_string( ss.str() ), coerce< utf8_string>( L"\
Content-Type: multipart/mixed; boundary=\"" + headers[L"Content-Type"].subvalue( L"boundary" ).value() + L"\"\r\n\
\r\n\
--" + headers[L"Content-Type"].subvalue( L"boundary" ).value() + L"\r\n\
Content-Length: 18\r\n\
Content-Transfer-Encoding: 8bit\r\n\
Content-Type: text/plain; charset=\"utf-8\"\r\n\
\r\n\
Test text document\r\n\
--" + headers[L"Content-Type"].subvalue( L"boundary" ).value() + L"--\r\n\
" ) );
    } catch ( fostlib::exceptions::exception &e ) {
        if ( headers[L"Content-Type"].subvalue( L"boundary" ).isnull() )
            e.info() << L"Output from the envelope:\n" << ss.str() << std::endl;
        throw;
    }
}

