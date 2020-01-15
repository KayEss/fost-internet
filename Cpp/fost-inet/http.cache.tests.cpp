/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/ua/cache>
#include <fost/test>


FSL_TEST_SUITE(http_cache);


FSL_TEST_FUNCTION(expect_get) {
    fostlib::url url;
    fostlib::ua::expect_get(url, fostlib::json{});
    FSL_CHECK_EQ(fostlib::ua::get_json(url), fostlib::json{});
}
