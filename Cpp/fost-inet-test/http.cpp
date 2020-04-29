/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet-test.hpp"
#include <fost/test>

#include <fost/http.useragent.hpp>
#include <fost/http.server.hpp>


using namespace std::chrono_literals;


FSL_TEST_SUITE(http);


FSL_TEST_FUNCTION(basic_request) {
    fostlib::http::server server{{127, 0, 0, 1}, 45634};
    std::thread{[&server]() {
        auto req = server();
        fostlib::text_body response{"OK"};
        (*req)(response);
    }}.detach();
    fostlib::http::user_agent ua{{}};
    auto const response = ua.get(fostlib::url{"http://127.0.0.1:45634"});
    FSL_CHECK_EQ(response->status(), 200);
}
