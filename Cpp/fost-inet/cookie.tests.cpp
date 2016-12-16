/*
    Copyright 2016 Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/test>
#include <fost/internet>


FSL_TEST_SUITE(cookie);


FSL_TEST_FUNCTION(empty) {
    fostlib::mime::mime_headers h;
    FSL_CHECK(not h.exists("Cookie"));
    h.parse("Cookie: ");
    fostlib::headers_base::content cookie(h["Cookie"]);
    FSL_CHECK(cookie.value().empty());
    FSL_CHECK(cookie.begin() == cookie.end());
}


FSL_TEST_FUNCTION(realistic) {
    const fostlib::string cv(
        "_cv=XX.12314.23123; "
        "logged_in=yes; "
        "dotcom_user=UsrEx; "
        "_ga=GA1.2.234234.43232; "
        "tz=Asia%2FBangkok; "
        "user_session=49lQUhQ1RHR8kB4G-%2FkiDIiNOCoKZtbV3; "
        "__Host-user_session_same_site=oinw9IjFkBOXhXaU-wX6XgHoBSo%2BU5V3E; "
        "_sess=P%2BpO%2BR%3D%3D-");
    fostlib::mime::mime_headers h;
    h.parse("Cookie: " + cv);
    fostlib::headers_base::content cookie(h["Cookie"]);
    /// Although we've parsed it, the generic header value parser doens't
    /// understand sub-values so it leaves that as a single string
    FSL_CHECK_EQ(cookie.value(), cv);
    FSL_CHECK(cookie.begin() == cookie.end());
    /// Pass it for parsing
    fostlib::parse_cookies(cookie);
    FSL_CHECK(cookie.value().empty());
    FSL_CHECK(cookie.begin() != cookie.end());
    FSL_CHECK_EQ(cookie.subvalue("_cv").value(), "XX.12314.23123");
    FSL_CHECK_EQ(cookie.subvalue("logged_in").value(), "yes");
    FSL_CHECK_EQ(cookie.subvalue("dotcom_user").value(), "UsrEx");
    FSL_CHECK_EQ(cookie.subvalue("_ga").value(), "GA1.2.234234.43232");
    FSL_CHECK_EQ(cookie.subvalue("tz").value(), "Asia/Bangkok");
    FSL_CHECK_EQ(cookie.subvalue("user_session").value(), "49lQUhQ1RHR8kB4G-/kiDIiNOCoKZtbV3");
    FSL_CHECK_EQ(cookie.subvalue("__Host-user_session_same_site").value(), "oinw9IjFkBOXhXaU-wX6XgHoBSo+U5V3E");
    FSL_CHECK_EQ(cookie.subvalue("_sess").value(), "P+pO+R==-");
}

