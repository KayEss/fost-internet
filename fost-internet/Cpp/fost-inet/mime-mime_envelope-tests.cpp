/*
    Copyright 2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/mime.hpp>

#include <boost/lambda/lambda.hpp>


using namespace fostlib;


FSL_TEST_SUITE( mime__mime_envelope );


FSL_TEST_FUNCTION( attach ) {
    mime_envelope e;
    e.attach<text_body>(fostlib::utf8_string("body 1"));
    e.attach<text_body>(fostlib::utf8_string("body 2"), mime::mime_headers());
    e.attach<text_body>(fostlib::utf8_string("<p>body 3</p>"),
        mime::mime_headers(), "text/html");
}


FSL_TEST_FUNCTION( iterator ) {
    mime_envelope e;
    std::size_t blocks = 0u;
    /*for (mime::const_iterator i(e.begin()); i != e.end(); ++i )
        blocks++;
    FSL_CHECK_EQ(blocks, 0u);*/
}
