#include "fost-inet-test.hpp"
#include <fost/http>


using namespace fostlib;


FSL_TEST_SUITE(user_agent);


FSL_TEST_FUNCTION(request) {
    FSL_CHECK_NOTHROW(http::user_agent::request r("HEAD", url()));
}


FSL_TEST_FUNCTION(user_agent) {
    http::user_agent ua(url("https://kirit.com/"));
    http::user_agent::request r("HEAD", ua.base());
    ua(r);
}


FSL_TEST_FUNCTION(mime_request) {
    std::shared_ptr<mime> request_body(new empty_mime);
    http::user_agent ua(url("https://kirit.com/"));
    http::user_agent::request r("GET", ua.base(), request_body);
    FSL_CHECK_NOTHROW(ua(r));
}


FSL_TEST_FUNCTION(connect_error) {
    setting<int64_t> c_connect_timeout(
            "fost-internet/Cpp/fost-inet-test/connection.cpp",
            "Network settings", "Connect time out", 1);
    http::user_agent ua;
    {
        http::user_agent::request r("GET", url("http://localhost:64546/"));
        FSL_CHECK_EXCEPTION(ua(r), exceptions::socket_error &);
    }
    {
        http::user_agent::request r("GET", url("http://10.45.234.124:64546/"));
        FSL_CHECK_EXCEPTION(ua(r), exceptions::socket_error &);
    }
}
