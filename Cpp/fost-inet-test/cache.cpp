/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet-test.hpp"
#include <fost/ua/cache>
#include <fost/ua/counters.hpp>
#include <fost/ua/exceptions.hpp>
#include <fost/test>


FSL_TEST_SUITE(http_cache);


FSL_TEST_FUNCTION(can_make_request) {
    auto const requests = fostlib::ua::p_network_requests.value();
    auto const errors = fostlib::ua::p_status_errors.value();

    auto const zzt = fostlib::ua::get_json(
            fostlib::url{"https://kirit.com/beanbag/gtf/ZZT"});
    FSL_CHECK(zzt.isarray());
    FSL_CHECK_EQ(requests + 1, fostlib::ua::p_network_requests.value());
    FSL_CHECK_EQ(errors, fostlib::ua::p_status_errors.value());

    FSL_CHECK_EXCEPTION(
            fostlib::ua::get_json(
                    fostlib::url{"https://kirit.com/beanbag/gtf/ZZ"}),
            fostlib::ua::resource_not_found &);
    FSL_CHECK_EQ(requests + 2, fostlib::ua::p_network_requests.value());
    FSL_CHECK_EQ(errors + 1, fostlib::ua::p_status_errors.value());
}


FSL_TEST_FUNCTION(can_get_strings) {
    auto const requests = fostlib::ua::p_network_requests.value();
    auto const errors = fostlib::ua::p_status_errors.value();
    auto const source =
            fostlib::ua::get_json(fostlib::url{"https://www.felspar.com/"});
    FSL_CHECK(fostlib::coerce<std::optional<f5::u8view>>(source).has_value());
    FSL_CHECK_EQ(requests + 1, fostlib::ua::p_network_requests.value());
    FSL_CHECK_EQ(errors, fostlib::ua::p_status_errors.value());
}


FSL_TEST_FUNCTION(cannot_get_image) {
    auto const requests = fostlib::ua::p_network_requests.value();
    auto const errors = fostlib::ua::p_status_errors.value();
    FSL_CHECK_EXCEPTION(
            fostlib::ua::get_json(
                    fostlib::url{"https://www.felspar.com/Felspar.png"}),
            fostlib::exceptions::unicode_encoding &);
    FSL_CHECK_EQ(requests + 1, fostlib::ua::p_network_requests.value());
    FSL_CHECK_EQ(errors, fostlib::ua::p_status_errors.value());
}


FSL_TEST_FUNCTION(forbidden) {
    auto const errors = fostlib::ua::p_status_errors.value();
    FSL_CHECK_EXCEPTION(
            fostlib::ua::get_json(
                    fostlib::url{"https://kirit.com/_resources/"}),
            fostlib::ua::forbidden &);
    FSL_CHECK_EQ(errors + 1, fostlib::ua::p_status_errors.value());
}


FSL_TEST_FUNCTION(unauthorized) {
    auto const errors = fostlib::ua::p_status_errors.value();
    FSL_CHECK_EXCEPTION(
            fostlib::ua::get_json(
                    fostlib::url{"https://svn.felspar.com/private/"}),
            fostlib::ua::unauthorized &);
    FSL_CHECK_EQ(errors + 1, fostlib::ua::p_status_errors.value());
}
