/*
    Copyright 2010-2014, Felspar Co Ltd. http://support.felspar.com/
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
    fostlib::headers_base::content c1,
        c2("applicaton/long_mime_type");
    c2.subvalue("long-sub-key1", "long key value with long value and a long value");
    c2.subvalue("long-sub-key2", "long key value with long value and a long value");
    ss << c1 << "\n" << c2;
}


FSL_TEST_FUNCTION(parse) {
    fostlib::mime::mime_headers h1, h2, h3;
    h1.parse("Header-Value: One Two");
    FSL_CHECK(h1.exists("header-value"));
    FSL_CHECK_EQ(h1["Header-Value"].value(), "One Two");

    h2.parse("Header-Value: One Two; two=2");
    FSL_CHECK(h1.exists("header-value"));
    FSL_CHECK_EQ(h1["header-value"].value(), "One Two");

    h3.parse("Header_Value: One Two");
    FSL_CHECK(not h3.exists("Header_Value"));
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


FSL_TEST_FUNCTION( json_content ) {
    headers_base::content line("Header value");
    FSL_CHECK_EQ(coerce<json>(line), json("Header value"));

    mime::mime_headers headers;
    FSL_CHECK_EQ(coerce<json>(headers).size(), 0u);
    FSL_CHECK_EQ(coerce<json>(headers), json::object_t());

    headers.add("H1", "Content 1");
    FSL_CHECK_EQ(coerce<json>(headers).size(), 1u);
    FSL_CHECK_EQ(coerce<json>(headers)["H1"], json("Content 1"));

    headers.add("H2", "Content 2");
    FSL_CHECK_EQ(coerce<json>(headers).size(), 2u);
    FSL_CHECK_EQ(coerce<json>(headers)["H1"].size(), 1u);
    FSL_CHECK_EQ(coerce<json>(headers)["H1"], json("Content 1"));
    FSL_CHECK_EQ(coerce<json>(headers)["H2"], json("Content 2"));

    headers.add("H1", "Content 1a");
    FSL_CHECK_EQ(coerce<json>(headers).size(), 2u);
    FSL_CHECK_EQ(coerce<json>(headers)["H1"].size(), 2u);
    FSL_CHECK_EQ(coerce<json>(headers)["H1"][0], json("Content 1"));
    FSL_CHECK_EQ(coerce<json>(headers)["H1"][1], json("Content 1a"));
    FSL_CHECK_EQ(coerce<json>(headers)["H2"], json("Content 2"));

    headers.add("H1", "Content 1b");
    FSL_CHECK_EQ(coerce<json>(headers).size(), 2u);
    FSL_CHECK_EQ(coerce<json>(headers)["H1"].size(), 3u);
    FSL_CHECK_EQ(coerce<json>(headers)["H1"][0], json("Content 1"));
    FSL_CHECK_EQ(coerce<json>(headers)["H1"][1], json("Content 1a"));
    FSL_CHECK_EQ(coerce<json>(headers)["H1"][2], json("Content 1b"));
    FSL_CHECK_EQ(coerce<json>(headers)["H2"], json("Content 2"));
}

