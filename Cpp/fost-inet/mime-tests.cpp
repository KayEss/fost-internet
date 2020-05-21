/**
    Copyright 2008-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet-test.hpp"
#include <fost/mime.hpp>

#include <fost/insert>


using namespace fostlib;


FSL_TEST_SUITE(mime);


FSL_TEST_FUNCTION(headers) {
    mime::mime_headers headers;
    headers.parse("X-First: value\r\nX-Second: value");
    FSL_CHECK(headers.exists("X-First"));
    FSL_CHECK(headers.exists("X-Second"));
    FSL_CHECK(!headers.exists("X-Third"));

    FSL_CHECK_EQ(headers["X-First"].value(), "value");
    FSL_CHECK_EQ(headers["X-Second"].value(), "value");

    FSL_CHECK_EQ(headers["x-first"].value(), "value");
    FSL_CHECK_EQ(headers["x-second"].value(), "value");

    headers.set("X-First", "Another value");
    FSL_CHECK_EQ(headers["X-First"].value(), "Another value");
}

FSL_TEST_FUNCTION(drops_underscores) {
    mime::mime_headers headers;
    headers.parse("X_First: value1\r\nX-Second: value2");

    FSL_CHECK(not headers.exists("X_First"));
    FSL_CHECK(headers.exists("X-Second"));
    FSL_CHECK(not headers.exists("X-Third"));
}

FSL_TEST_FUNCTION(headers_case_insensitive) {
    mime::mime_headers headers;
    headers.set("ETag", "somevalue");
    FSL_CHECK_EQ(headers["etag"].value(), "somevalue");
    headers.set("Etag", "newvalue");
    FSL_CHECK_EQ(headers["etag"].value(), "newvalue");
    std::stringstream ss;
    ss << headers;
    FSL_CHECK_EQ(ss.str(), "Etag: newvalue\r\n");
}

FSL_TEST_FUNCTION(subvalues_case_insensitive) {
    mime::mime_headers headers;
    headers.set("Content-Type", "text/plain");
    headers.set_subvalue("content-type", "encoding", "utf-8");
    FSL_CHECK(headers["Content-type"].subvalue("Encoding"));
    FSL_CHECK_EQ(headers["Content-type"].subvalue("Encoding").value(), "utf-8");
    std::stringstream ss;
    ss << headers;
    FSL_CHECK_EQ(ss.str(), "Content-Type: text/plain; encoding=\"utf-8\"\r\n");
}

FSL_TEST_FUNCTION(headers_without_values_are_legit) {
    mime::mime_headers headers;
    // from an Exchange bounced message:
    headers.parse("X-MS-TNEF-Correlator: \r\n");
    headers.set("TE");

    std::ostringstream ss;
    ss << headers;
    FSL_CHECK_EQ("TE: \r\nX-MS-TNEF-Correlator: \r\n", ss.str());
}

FSL_TEST_FUNCTION(empty_mime_output) {
    empty_mime empty;
    std::stringstream ss;
    ss << empty;
    FSL_CHECK_EQ(
            ss.str(),
            "\
Content-Length: 0\r\n\
Content-Type: application/x-empty\r\n\
\r\n\
");
}
FSL_TEST_FUNCTION(empty_mime_as_string) {
    empty_mime em;
    FSL_CHECK_EQ(coerce<string>(em), string());
    FSL_CHECK_EQ(em.body_as_string(), string());
}

FSL_TEST_FUNCTION(empty_mime_with_headers) {
    empty_mime empty;
    empty.headers().set("Host", fostlib::string("localhost"));
    empty.headers().set("User-Agent", fostlib::string("Fake agent"));
    std::stringstream ss;
    ss << empty;
    FSL_CHECK_EQ(
            ss.str(),
            "\
Content-Length: 0\r\n\
Content-Type: application/x-empty\r\n\
Host: localhost\r\n\
User-Agent: Fake agent\r\n\
\r\n\
");
}
FSL_TEST_FUNCTION(empty_mime_iterators) {
    empty_mime empty;
    FSL_CHECK(empty.begin() == empty.end());
}

FSL_TEST_FUNCTION(text1) {
    text_body ta("Test text document");
    std::stringstream ss;
    ss << ta;
    mime::mime_headers headers;
    headers.parse(
            coerce<string>(partition(utf8_string(ss.str()), "\r\n\r\n").first));
    FSL_CHECK_EQ(
            utf8_string(ss.str()),
            "\
Content-Length: 18\r\n\
Content-Transfer-Encoding: 8bit\r\n\
Content-Type: text/plain; charset=\"utf-8\"\r\n\
\r\n\
Test text document");
}
FSL_TEST_FUNCTION(text2) {
    text_body ta(utf8_string("Test text document"));
    std::stringstream ss;
    ss << ta;
    mime::mime_headers headers;
    headers.parse(
            coerce<string>(partition(utf8_string(ss.str()), "\r\n\r\n").first));
    FSL_CHECK_EQ(
            utf8_string(ss.str()),
            "\
Content-Length: 18\r\n\
Content-Transfer-Encoding: 8bit\r\n\
Content-Type: text/plain; charset=\"utf-8\"\r\n\
\r\n\
Test text document");
}
FSL_TEST_FUNCTION(text4_unicode) {
    text_body ta(utf8_string("Test text document\xE2\x80\xBD"));
    std::stringstream ss;
    ss << ta;
    mime::mime_headers headers;
    headers.parse(
            coerce<string>(partition(utf8_string(ss.str()), "\r\n\r\n").first));
    FSL_CHECK_EQ(
            utf8_string(ss.str()),
            "\
Content-Length: 21\r\n\
Content-Transfer-Encoding: 8bit\r\n\
Content-Type: text/plain; charset=\"utf-8\"\r\n\
\r\n\
Test text document\xE2\x80\xBD");
    FSL_CHECK_EQ(
            ta.body_as_string(), f5::u8view{"Test text document\xE2\x80\xBD"});
}
FSL_TEST_FUNCTION(text_iterators) {
    text_body ta(utf8_string("Test text document"));
    FSL_CHECK(ta.begin() != ta.end());
    FSL_CHECK_EQ(
            fostlib::string("Test text document"),
            std::string(
                    reinterpret_cast<const char *>((*ta.begin()).first),
                    reinterpret_cast<const char *>((*ta.begin()).second)));
}
FSL_TEST_FUNCTION(text_as_string) {
    text_body ta(utf8_string("Test text document"));
    FSL_CHECK_EQ(coerce<string>(ta), "Test text document");
}


FSL_TEST_FUNCTION(mime_attachment) {
    mime_envelope envelope;
    envelope.items().push_back(
            boost::shared_ptr<mime>(new text_body("Test text document")));
    envelope.boundary();
    std::stringstream ss;
    ss << envelope;
    mime::mime_headers headers;

    FSL_CHECK_NOTHROW(headers.parse(coerce<string>(
            partition(utf8_string(ss.str()), "\r\n\r\n").first)));
    try {
        FSL_CHECK_EQ(
                utf8_string(ss.str()),
                coerce<utf8_string>(
                        "\
Content-Type: multipart/mixed;\r\n\
 boundary=\"" + headers["Content-Type"].subvalue("boundary").value()
                        + "\"\r\n\
\r\n\
--" + headers["Content-Type"].subvalue("boundary").value()
                        + "\r\n\
Content-Length: 18\r\n\
Content-Transfer-Encoding: 8bit\r\n\
Content-Type: text/plain; charset=\"utf-8\"\r\n\
\r\n\
Test text document\r\n\
--" + headers["Content-Type"].subvalue("boundary").value()
                        + "--\r\n\
"));
    } catch (exceptions::exception &e) {
        if (not headers["Content-Type"].subvalue("boundary")) {
            insert(e.data(), "envelope", ss.str().c_str());
        }
        throw;
    }
}

FSL_TEST_FUNCTION(wrap_long_mime_header) {
    text_body mail("A short message\n\nanother line\n");
    mail.headers().set("Subject", "Test email");
    mail.headers().set(
            "Dummy",
            "This is a long header for email which should be 'folded' "
            "according to RFC5322. The folding must happen right before "
            "whitespaces around the 78th position.");

    std::stringstream ss;
    ss << mail.headers();

    FSL_CHECK_EQ(
            ss.str(),
            "Content-Length: 30\r\n"
            "Content-Transfer-Encoding: 8bit\r\n"
            "Content-Type: text/plain; charset=\"utf-8\"\r\n"
            "Dummy: This is a long header for email which should be 'folded' "
            "according to\r\n"
            " RFC5322. The folding must happen right before whitespaces around "
            "the 78th\r\n"
            " position.\r\n"
            "Subject: Test email\r\n");
}

FSL_TEST_FUNCTION(wrap_78_char_long_mime_header) {
    text_body mail("A short message\n\nanother line\n");
    mail.headers().set("Subject", "Test email");
    mail.headers().set(
            "Dummy",
            "This is a 78-char long header for eachi that should not be "
            "folded. :)");

    std::stringstream ss;
    ss << mail.headers();

    FSL_CHECK_EQ(
            ss.str(),
            "Content-Length: 30\r\n"
            "Content-Transfer-Encoding: 8bit\r\n"
            "Content-Type: text/plain; charset=\"utf-8\"\r\n"
            "Dummy: This is a 78-char long header for eachi that should not be "
            "folded. :)\r\n"
            "Subject: Test email\r\n");
}

FSL_TEST_FUNCTION(wrap_empty_mime_header) {
    text_body mail("A short message\n\nanother line\n");
    mail.headers().set("Subject", "Test email");
    mail.headers().set("Dummy", "");

    std::stringstream ss;
    ss << mail.headers();

    FSL_CHECK_EQ(
            ss.str(),
            "Content-Length: 30\r\n"
            "Content-Transfer-Encoding: 8bit\r\n"
            "Content-Type: text/plain; charset=\"utf-8\"\r\n"
            "Dummy: \r\n"
            "Subject: Test email\r\n");
}
