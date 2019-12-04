/**
    Copyright 2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/http>
#include <fost/log>
#include <fost/test>


FSL_TEST_SUITE(tls);


FSL_TEST_FUNCTION(defaults) {
    fostlib::http::user_agent ua;
    auto const response = ua.get(fostlib::url{"https://sha256.badssl.com/"});
    FSL_CHECK_EQ(response->status(), 200);
    FSL_CHECK_EXCEPTION(
            ua.get(fostlib::url{"https://self-signed.badssl.com/"}),
            fostlib::exceptions::socket_error &);
}


FSL_TEST_FUNCTION(no_default_paths) {
    fostlib::setting<bool> const no_default_paths{
            "fost-inet-test/tls.cpp", fostlib::c_tls_use_standard_verify_paths,
            false};
    fostlib::http::user_agent ua;
    FSL_CHECK_EXCEPTION(
            ua.get(fostlib::url{"https://sha256.badssl.com/"}),
            fostlib::exceptions::socket_error &);
}


FSL_TEST_FUNCTION(specified_digicert_leaf) {
    fostlib::setting<bool> const no_default_paths{
            "fost-inet-test/tls.cpp", fostlib::c_tls_use_standard_verify_paths,
            false};
    fostlib::setting<fostlib::json> const digicert_ca{
            "fost-inet-test/tls.cpp", fostlib::c_extra_leaf_certificates,
            fostlib::json::array_t{{fostlib::digicert_root_ca()}}};
    fostlib::http::user_agent ua;
    auto const response = ua.get(fostlib::url{"https://sha256.badssl.com/"});
    FSL_CHECK_EQ(response->status(), 200);
}


FSL_TEST_FUNCTION(specify_lets_encrypt_leaf) {
    fostlib::setting<bool> const no_default_paths{
            "fost-inet-test/tls.cpp", fostlib::c_tls_use_standard_verify_paths,
            false};
    fostlib::setting<fostlib::json> const digicert_ca{
            "fost-inet-test/tls.cpp", fostlib::c_extra_leaf_certificates,
            fostlib::json::array_t{{fostlib::lets_encrypt_root()}}};
    fostlib::http::user_agent ua;
    auto const response =
            ua.get(fostlib::url{"https://valid-isrgrootx1.letsencrypt.org/"});
    FSL_CHECK_EQ(response->status(), 200);
}


FSL_TEST_FUNCTION(specify_multiple_roots) {
    auto logger = fostlib::log::debug(fostlib::c_fost_inet);
    fostlib::setting<bool> const no_default_paths{
            "fost-inet-test/tls.cpp", fostlib::c_tls_use_standard_verify_paths,
            false};
    fostlib::setting<fostlib::json> const digicert_ca{
            "fost-inet-test/tls.cpp", fostlib::c_extra_leaf_certificates,
            fostlib::json::array_t{{fostlib::digicert_root_ca()},
                                   {fostlib::lets_encrypt_root()}}};
    fostlib::http::user_agent ua;
    {
        auto const response =
                ua.get(fostlib::url{"https://sha256.badssl.com/"});
        logger("sha256.badssl.com", response->status());
        FSL_CHECK_EQ(response->status(), 200);
    }
    {
        auto const response = ua.get(
                fostlib::url{"https://valid-isrgrootx1.letsencrypt.org/"});
        logger("valid-isrgrootx1.letsencrypt.org", response->status());
        FSL_CHECK_EQ(response->status(), 200);
    }
    /// This host is used for web proxy tests. It doesn't work because although
    /// it uses Let's Encrypt, they sign with an intermediate and this setting
    /// doesn't do chain verification.
    FSL_CHECK_EXCEPTION(
            ua.get(fostlib::url{"https://kirit.com/"}),
            fostlib::exceptions::socket_error &);
}


FSL_TEST_FUNCTION(specify_certificate_verification_file) {
    auto logger = fostlib::log::debug(fostlib::c_fost_inet);
    fostlib::setting<bool> const no_default_paths{
            "fost-inet-test/tls.cpp",
            fostlib::c_tls_use_standard_verify_paths, false};
    fostlib::http::user_agent ua;

    FSL_CHECK_EXCEPTION(
            ua.get(fostlib::url{"https://kirit.com/"}),
            fostlib::exceptions::socket_error &);

    /// TODO Add verification file setting to known file
    auto const response = ua.get(fostlib::url{"https://kirit.com/"});
    logger("kirit.com", response->status());
    FSL_CHECK_EQ(response->status(), 200);
}
