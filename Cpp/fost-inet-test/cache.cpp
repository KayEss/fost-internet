/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet-test.hpp"
#include <fost/ua/cache>
#include <fost/ua/exceptions.hpp>
#include <fost/test>


FSL_TEST_SUITE(http_cache);


FSL_TEST_FUNCTION(can_make_request) {
    auto const zzt = fostlib::ua::get_json(
            fostlib::url{"https://kirit.com/beanbag/gtf/ZZT"});
    FSL_CHECK(zzt.isarray());

    FSL_CHECK_EXCEPTION(
            fostlib::ua::get_json(
                    fostlib::url{"https://kirit.com/beanbag/gtf/ZZ"}),
            fostlib::ua::resource_not_found &);
}
