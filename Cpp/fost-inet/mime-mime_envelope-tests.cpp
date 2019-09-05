/**
    Copyright 2010-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet-test.hpp"
#include <fost/mime.hpp>


using namespace fostlib;


FSL_TEST_SUITE(mime__mime_envelope);


FSL_TEST_FUNCTION(attach) {
    mime_envelope e;
    e.attach<text_body>(fostlib::utf8_string("body 1"));
    e.attach<text_body>(fostlib::utf8_string("body 2"), mime::mime_headers());
    e.attach<text_body>(
            fostlib::utf8_string("<p>body 3</p>"), mime::mime_headers(),
            "text/html");
}


FSL_TEST_FUNCTION(iterator) {
    { // With content
        mime_envelope e;
        e.attach<text_body>(fostlib::utf8_string("body"));
        std::size_t blocks = 0u;
        for (mime::const_iterator i(e.begin()); i != e.end(); ++i) blocks++;
        FSL_CHECK_EQ(blocks, 4u);
    }
}
