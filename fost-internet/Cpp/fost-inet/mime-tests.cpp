/*
    Copyright 2008-2011, Felspar Co Ltd. http://support.felspar.com/
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

FSL_TEST_FUNCTION( empty_mime_output ) {
    empty_mime empty;
    std::stringstream ss;
    ss << empty;
    FSL_CHECK_EQ( ss.str(), "\
Content-Length: 0\r\n\
Content-Type: application/x-empty\r\n\
\r\n\
" );
}
FSL_TEST_FUNCTION( empty_mime_as_string ) {
    FSL_CHECK_EQ(coerce<string>(empty_mime()), string());
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
FSL_TEST_FUNCTION( empty_mime_iterators ) {
    empty_mime empty;
    FSL_CHECK( empty.begin() == empty.end() );
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
FSL_TEST_FUNCTION(text_iterators) {
    text_body ta(utf8_string("Test text document"));
    FSL_CHECK( ta.begin() != ta.end() );
    FSL_CHECK_EQ(fostlib::string("Test text document"), fostlib::string(
        reinterpret_cast<const char *>((*ta.begin()).first),
        reinterpret_cast<const char *>((*ta.begin()).second)
    ));
}
FSL_TEST_FUNCTION(text_as_string) {
    text_body ta(utf8_string("Test text document"));
    FSL_CHECK_EQ(coerce<string>(ta), "Test text document");
}


FSL_TEST_FUNCTION( mime_attachment ) {
    mime_envelope envelope;
    envelope.items().push_back(
        boost::shared_ptr< mime >( new text_body( L"Test text document" ) ));
    envelope.boundary();
    std::stringstream ss;
    ss << envelope;
    mime::mime_headers headers;

    FSL_CHECK_NOTHROW(headers.parse(
        coerce< string >( partition( utf8_string( ss.str() ), "\r\n\r\n" ).first )));
    try {
        FSL_CHECK_EQ( utf8_string( ss.str() ), coerce< utf8_string>( L"\
Content-Type: multipart/mixed;\r\n\
 boundary=\"" + headers[L"Content-Type"].subvalue( L"boundary" ).value() + L"\"\r\n\
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

FSL_TEST_FUNCTION( wrap_long_mime_header ) {
    text_body mail( L"A short message\n\nanother line\n" );
    mail.headers().set(L"Subject", L"Test email");
    mail.headers().set(L"Dummy", L"This is a long header for email which should be 'folded' according to RFC5322. The folding must happen right before whitespaces around the 78th position.");

    std::stringstream ss;
    ss << mail.headers();

    FSL_CHECK_EQ(ss.str(), "Content-Length: 30\r\n"
                           "Content-Transfer-Encoding: 8bit\r\n"
                           "Content-Type: text/plain; charset=\"utf-8\"\r\n"
                           "Dummy: This is a long header for email which should be 'folded' according to\r\n"
                           " RFC5322. The folding must happen right before whitespaces around the 78th\r\n"
                           " position.\r\n"
                           "Subject: Test email\r\n"
            );
}

FSL_TEST_FUNCTION( wrap_78_char_long_mime_header ) {
    text_body mail( L"A short message\n\nanother line\n" );
    mail.headers().set(L"Subject", L"Test email");
    mail.headers().set(L"Dummy", L"This is a 78-char long header for eachi that should not be folded. :)");

    std::stringstream ss;
    ss << mail.headers();

    FSL_CHECK_EQ(ss.str(), "Content-Length: 30\r\n"
                           "Content-Transfer-Encoding: 8bit\r\n"
                           "Content-Type: text/plain; charset=\"utf-8\"\r\n"
                           "Dummy: This is a 78-char long header for eachi that should not be folded. :)\r\n"
                           "Subject: Test email\r\n"
            );
}

FSL_TEST_FUNCTION( wrap_empty_mime_header ) {
    text_body mail( L"A short message\n\nanother line\n" );
    mail.headers().set(L"Subject", L"Test email");
    mail.headers().set(L"Dummy", L"");

    std::stringstream ss;
    ss << mail.headers();

    FSL_CHECK_EQ(ss.str(), "Content-Length: 30\r\n"
                           "Content-Transfer-Encoding: 8bit\r\n"
                           "Content-Type: text/plain; charset=\"utf-8\"\r\n"
                           "Dummy: \r\n"
                           "Subject: Test email\r\n"
            );
}
