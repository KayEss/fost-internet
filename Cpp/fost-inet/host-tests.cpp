/*
    Copyright 2010-2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/parse/host.hpp>
#include <fost/exception/parse_error.hpp>


using namespace fostlib;


FSL_TEST_SUITE( host );


FSL_TEST_FUNCTION(coerce_string) {
    fostlib::host h0 = fostlib::coerce<fostlib::host>(fostlib::string("localhost"));
    FSL_CHECK_EQ(h0.name(), "localhost");
    FSL_CHECK(not h0.service());

    fostlib::host h1 = fostlib::coerce<fostlib::host>(fostlib::string("www.example.com"));
    FSL_CHECK_EQ(h1.name(), "www.example.com");
    FSL_CHECK(not h1.service());

    fostlib::host h2 = fostlib::coerce<fostlib::host>(fostlib::string("localhost:80"));
    FSL_CHECK_EQ(h2.name(), "localhost");
    FSL_CHECK(h2.service());
    FSL_CHECK_EQ(h2.service().value(), "80");
    FSL_CHECK_EQ(fostlib::coerce< fostlib::string >(h2), "localhost:80");

    fostlib::host h3 = fostlib::coerce<fostlib::host>(fostlib::string("2130706434"));
    FSL_CHECK_EQ(h3.name(), "127.0.0.2");

    fostlib::host h4 = fostlib::coerce<fostlib::host>(fostlib::string("127.0.0.2"));
    FSL_CHECK_EQ(h3.name(), "127.0.0.2");
}


FSL_TEST_FUNCTION(normalise_case) {
    FSL_CHECK_EQ(fostlib::host("LOCALHOST").name(), "localhost");
    FSL_CHECK_EQ(fostlib::host("LOCALHOST", fostlib::string("http")).name(), "localhost");
    FSL_CHECK_EQ(fostlib::host("LOCALHOST", 80).name(), "localhost");
}
