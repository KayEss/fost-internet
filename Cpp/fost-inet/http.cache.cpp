/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet.hpp"
#include <fost/ua/cache.hpp>


fostlib::json fostlib::ua::get_json(url const &, headers const &) {
    return json{};
}


void fostlib::ua::expect_get(url const &, json, headers const &) {}
