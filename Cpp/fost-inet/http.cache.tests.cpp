/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/ua/cache>
#include <fost/ua/cache.detail.hpp>
#include <fost/test>


FSL_TEST_SUITE(http_cache);


FSL_TEST_FUNCTION(expect_get) {
    fostlib::url url;
    fostlib::ua::expect_get(url, fostlib::json{});
    FSL_CHECK_EQ(fostlib::ua::get_json(url), fostlib::json{});
}


/// ## Implementation details


FSL_TEST_FUNCTION(cache_keys) {
    FSL_CHECK_EQ(
            fostlib::ua::cache_key(
                    "GET", fostlib::url{}, fostlib::ua::headers{}),
            "964dyfcyk487v6wzdsajacmxkj0zrgc6mk1bdmprd3vzjvd83h90");
}
