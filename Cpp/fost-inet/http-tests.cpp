/*
    Copyright 2009-2018, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/http.server.hpp>


using namespace fostlib;


FSL_TEST_SUITE(http_server);


FSL_TEST_FUNCTION(mock_no_handler) {
    http::server::request req(
            "GET", url::filepath_string("/"),
            std::unique_ptr<binary_body>(new binary_body));
    empty_mime response;
    FSL_CHECK_EXCEPTION(req(response), exceptions::null &);
}
namespace {
    void handler(bool &called, const mime &, const ascii_string &) {
        called = true;
    }
}
FSL_TEST_FUNCTION(mock_with_handler) {
    bool called = false;
    http::server::request req(
            "GET", "/", std::unique_ptr<binary_body>(new binary_body),
            [&called](const mime &m, const ascii_string &a) {
                handler(called, m, a);
            });
    empty_mime response;
    req(response);
    FSL_CHECK(called);
}


FSL_TEST_FUNCTION(request_get) {
    http::user_agent::request get("GET", url());
    FSL_CHECK(get.data().begin() == get.data().end());
}
FSL_TEST_FUNCTION(request_post) {
    http::user_agent::request post1("POST", url());
    FSL_CHECK(post1.data().begin() == post1.data().end());

    http::user_agent::request post2("POST", url(), string("body data"));
    FSL_CHECK(post2.data().begin() != post2.data().end());
    FSL_CHECK_EQ(
            fostlib::string("body data"),
            fostlib::string(
                    reinterpret_cast<const char *>(
                            (*post2.data().begin()).first),
                    reinterpret_cast<const char *>(
                            (*post2.data().begin()).second)));
    // Ensure the first pass didn't consume the data
    FSL_CHECK_EQ(
            fostlib::string("body data"),
            fostlib::string(
                    reinterpret_cast<const char *>(
                            (*post2.data().begin()).first),
                    reinterpret_cast<const char *>(
                            (*post2.data().begin()).second)));
}
