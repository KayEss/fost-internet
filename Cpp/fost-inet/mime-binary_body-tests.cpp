/**
    Copyright 2010-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet-test.hpp"
#include <fost/mime.hpp>
#include <fost/exception/unicode_encoding.hpp>


using namespace fostlib;


FSL_TEST_SUITE(mime__binary_body);


FSL_TEST_FUNCTION(constructor) {
    std::vector<unsigned char> data(256, 123);
    binary_body body(data);
    FSL_CHECK(body.data() == data);
}


FSL_TEST_FUNCTION(iterator) {
    std::vector<unsigned char> data(256, 123);
    binary_body body(data);
    const_memory_block block = *body.begin();
    FSL_CHECK_EQ(bytes(block), 256u);
    for (const unsigned char *c(
                 reinterpret_cast<const unsigned char *>(block.first));
         c != block.second; ++c)
        FSL_CHECK_EQ(*c, 123);
}


FSL_TEST_FUNCTION(convert_to_text_empty) {
    std::vector<unsigned char> data(256, 0);
    binary_body body(data);
    FSL_CHECK_EQ(coerce<utf8_string>(body), coerce<utf8_string>(data));
}


FSL_TEST_FUNCTION(convert_to_text_fails) {
    std::vector<unsigned char> data(256, 0x80);
    binary_body body(data);
    FSL_CHECK_EXCEPTION(
            coerce<string>(body), fostlib::exceptions::unicode_encoding &);
}


FSL_TEST_FUNCTION(construct_from_byte_array) {
    std::vector<char> data(256, 0x20);
    binary_body body(&data[0], &data[0] + data.size());
    FSL_CHECK_EQ(coerce<string>(body), string(256, ' '));
}


FSL_TEST_FUNCTION(construct_from_signed_byte_array) {
    std::vector<signed char> data(256, 0x20);
    binary_body body(&data[0], &data[0] + data.size());
    FSL_CHECK_EQ(coerce<string>(body), string(256, ' '));
}


FSL_TEST_FUNCTION(construct_from_unsigned_byte_array) {
    std::vector<unsigned char> data(256, 0x20);
    binary_body body(&data[0], &data[0] + data.size());
    FSL_CHECK_EQ(coerce<string>(body), string(256, ' '));
}
