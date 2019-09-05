/**
    Copyright 2008-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet-test.hpp"
#include <fost/parse/url.hpp>
#include <fost/exception/parse_error.hpp>


using namespace fostlib;


FSL_TEST_SUITE(internet_url);


FSL_TEST_FUNCTION(filepath_string) {
    FSL_CHECK_NOTHROW(url::filepath_string a("a/bc.html"));
    FSL_CHECK_EXCEPTION(
            url::filepath_string a("a/b c.html"),
            fostlib::exceptions::parse_error);
    FSL_CHECK_EXCEPTION(
            url::filepath_string a("a/b%"), fostlib::exceptions::parse_error);
    FSL_CHECK_EXCEPTION(
            url::filepath_string a("a/b%%"), fostlib::exceptions::parse_error);
    FSL_CHECK_EXCEPTION(
            url::filepath_string a("a/b%2"), fostlib::exceptions::parse_error);
    FSL_CHECK_NOTHROW(url::filepath_string a("a/bc%2B.html"));

    FSL_CHECK_EQ(
            coerce<url::filepath_string>(string(L"abc")),
            url::filepath_string("abc"));
    FSL_CHECK_EQ(
            coerce<url::filepath_string>(string(L"a/bc.html")),
            url::filepath_string("a/bc.html"));
    FSL_CHECK_EQ(
            coerce<url::filepath_string>(string(L"a/b(c).html")),
            url::filepath_string("a/b%28c%29.html"));

    FSL_CHECK_EQ(coerce<fostlib::fs::path>(url::filepath_string("a")), L"a");
    FSL_CHECK_EQ(coerce<fostlib::fs::path>(url::filepath_string("%2B")), L"+");
}


FSL_TEST_FUNCTION(query_string) {
    url::query_string q1, q2;
    FSL_CHECK(not q1.as_string());
    FSL_CHECK(not q2.as_string());
    FSL_CHECK_EQ(
            q1.as_string().value_or(ascii_printable_string()),
            q2.as_string().value_or(ascii_printable_string()));
    FSL_CHECK(not q1["not-a-key"]);
    FSL_CHECK_EQ(q2.at("not-a-key").size(), 0u);
    q1 = q2;
    q1.append(L"key", null);
    FSL_CHECK_EQ(q1.as_string().value(), ascii_printable_string("key="));
    FSL_CHECK(not q1["key"]);
    FSL_CHECK_EQ(q1.has_key("key"), 1u);
    q1.append(L"key", null);
    FSL_CHECK_EQ(q1.as_string().value(), ascii_printable_string("key=&key="));
    q2 = q1;
    FSL_CHECK_EQ(q2.as_string().value(), ascii_printable_string("key=&key="));
    q1.append(L"key", L"(.)");
    FSL_CHECK_EQ(
            q1.as_string().value(),
            ascii_printable_string("key=&key=&key=%28.%29"));
    FSL_CHECK_EQ(q2.as_string().value(), ascii_printable_string("key=&key="));
    q2.append(L"key", L"\x2014");
    FSL_CHECK_EQ(
            q1.as_string().value(),
            ascii_printable_string("key=&key=&key=%28.%29"));
    FSL_CHECK_EQ(
            q2.as_string().value(),
            ascii_printable_string("key=&key=&key=%E2%80%94"));
}


FSL_TEST_FUNCTION(url) {
    FSL_CHECK_EQ(url().port(), 80);
    FSL_CHECK_EQ(
            url().as_string(), ascii_printable_string("http://localhost/"));
    FSL_CHECK_EQ(
            url(url("https://localhost/"), "/path").as_string(),
            ascii_printable_string("https://localhost/path"));
}


FSL_TEST_FUNCTION(query_string_parser) {
    auto check = [](string s) {
        url::query_string into;
        auto pos = s.begin();
        FSL_CHECK(query_string_p(pos, s.end(), into));
        FSL_CHECK_EQ(string(pos, s.end()), string());
        if (s.empty()) {
            FSL_CHECK(not into.as_string().has_value());
        } else {
            FSL_CHECK(into.as_string().has_value());
            FSL_CHECK_EQ(coerce<string>(into.as_string().value()), s);
        }
        return into;
    };
    auto check_p = [&check](
                           fostlib::string s, fostlib::string k,
                           fostlib::nullable<fostlib::string> v) {
        auto qs = check(s);
        FSL_CHECK(qs.has_key(k));
        if (v) {
            FSL_CHECK(qs[k]);
            FSL_CHECK_EQ(qs[k].value(), v.value());
        } else {
            FSL_CHECK(not qs[k]);
        }
        return qs;
    };

    check("");
    check_p("key=value", "key", "value");
    FSL_CHECK_EQ(
            check_p("key=value&key=value", "key", "value").at("key").size(),
            2u);
    check_p("key=", "key", fostlib::null);
    check_p("__=", "__", fostlib::null);
    check_p("key=&key=", "key", fostlib::null);
    check_p("key=value&key=", "key", "value");
    check_p("next=/path/", "next", "/path/");
    check("key1&key2");
    check("key2&key1"); // Ensure order is preserved
    check_p("key1=value1&key2=value=2&key3=value3", "key2", "value=2");
    check_p("key=%26%2312296%3B", "key", "&#12296;");
}


FSL_TEST_FUNCTION(url_parser_protocol) {
    FSL_CHECK_EQ(
            url("http://localhost/").protocol(),
            ascii_printable_string("http"));
    FSL_CHECK_EQ(url("http://localhost/").port(), 80);
    FSL_CHECK_EQ(
            url("https://localhost/").protocol(),
            ascii_printable_string("https"));
    FSL_CHECK_EQ(url("https://localhost/").port(), 443);
}


FSL_TEST_FUNCTION(url_parser_hostpart) {
    auto check = [](auto s, const url &u, std::string fails = std::string(),
                    bool works = true) {
        string str(s);
        auto pos = str.begin();
        url into;
        FSL_CHECK(fostlib::url_hostpart_p(pos, str.end(), into) == works);
        FSL_CHECK_EQ(string(pos, str.end()), fails);
        FSL_CHECK_EQ(u.as_string(), into.as_string());
    };

    check("http://localhost", url());
    check("http://127.0.0.1", url(host(127, 0, 0, 1)));
    check("http://10.0.2.2", url(host(10, 0, 2, 2)));
    check("http://www.felspar.com", url(host("www.felspar.com")));
    check("http://urquell-fn.appspot.com", url(host("urquell-fn.appspot.com")));
    check("http://123.45", url(host("123.45")));
    check("http://12345", url(host(12345)));
    check("http://localhost:80", url(host("localhost", "80")));
    check("http://localhost:8080", url(host("localhost", "8080")));
    check("https://localhost:80", url("https", host("localhost", "80")));

    check("http://www..felspar.com/", url(host("www")), "..felspar.com/");
    check("http://www./", url(host("www")), "./");
    check("http://.www/", url(), "http://.www/", false);
}


FSL_TEST_FUNCTION(url_parser_filespec) {
    auto check = [](string s, auto p) {
        ascii_printable_string into;
        auto pos = s.begin();
        FSL_CHECK(url_filespec_p(pos, s.end(), into));
        FSL_CHECK_EQ(string(pos, s.end()), string());
        FSL_CHECK_EQ(coerce<ascii_printable_string>(s), into);
    };

    check("/", "/");
    check("/*/", "/*/");
    check("/file.html", "/file.html");
    check("/Site:/file.html", "/Site:/file.html");
    check("/Site:/(file).html", "/Site:/(file).html");
    check("/Type/List:/Article%20(FSLib::::Content::::Article)",
          "/Type/List:/Article%20(FSLib::::Content::::Article)");
    check("/M&M", "/M&M");
    check("/$1", "/$1");
    check("/update/M%26M", "/update/M%26M");
}


FSL_TEST_FUNCTION(path_spec) {
    url u(L"http://localhost/");
    u.pathspec(url::filepath_string("/file-name"));
    FSL_CHECK_EQ(
            u.as_string(),
            ascii_printable_string("http://localhost/file-name"));

    FSL_CHECK_EQ(
            coerce<url::filepath_string>(fostlib::fs::path(L"test")),
            url::filepath_string("test"));
    FSL_CHECK_EQ(coerce<string>(u.pathspec()), "/file-name");
    u.pathspec(url::filepath_string("/Coups%20d%27%C3%A9tat"));
    FSL_CHECK_EQ(coerce<string>(u.pathspec()), L"/Coups d'\u00e9tat");

    url::filepath_string str("/Coups%20d%27%C3%A9tat");
    FSL_CHECK_EQ(coerce<string>(str), L"/Coups d'\u00e9tat");

    // This test is not reliable :(
    // #if BOOST_VERSION_MAJOR < 64
    //     FSL_CHECK_EXCEPTION(coerce<fostlib::fs::path>(str),
    //     std::exception&);
    // #else
    //     FSL_CHECK_EQ(coerce<fostlib::fs::path>(str), L"/Coups
    //     d'\u00e9tat");
    // #endif
}


#define TEST_PATH_SPEC_ENCODING(from, to) \
    FSL_CHECK_EQ( \
            url::filepath_string( \
                    ascii_printable_string(from), \
                    url::filepath_string::unencoded) \
                    .underlying(), \
            to)
FSL_TEST_FUNCTION(path_spec_encoding) {
    TEST_PATH_SPEC_ENCODING("invalid@felspar.com", "invalid%40felspar.com");
    TEST_PATH_SPEC_ENCODING("/@~:.-_+", "/%40~:.-_%2B");
}


FSL_TEST_FUNCTION(parse) {
    FSL_CHECK_NOTHROW(url a("http://localhost/");
                      FSL_CHECK_EQ(a.server().name(), L"localhost");)
    FSL_CHECK_EQ(url("http://localhost").server().name(), L"localhost");
    FSL_CHECK_EQ(
            url("http://localhost/file-path.html").pathspec(),
            url::filepath_string("/file-path.html"));
    FSL_CHECK_EQ(
            url("http://localhost:6789/file-path.html")
                    .server()
                    .service()
                    .value(),
            "6789");
    FSL_CHECK_EQ(url("http://localhost:6789/file-path.html").port(), 6789);

    FSL_CHECK_EXCEPTION(
            url("http://localhost/file path.html"),
            fostlib::exceptions::parse_error &);
    FSL_CHECK_EXCEPTION(
            url("http://localhost/file\\path.html"),
            fostlib::exceptions::parse_error &);

    FSL_CHECK_EQ(
            url("http://bmf.miro.felspar.net:8000/rest/email/new_subscription/"
                "123821/")
                    .pathspec(),
            url::filepath_string("/rest/email/new_subscription/123821/"));
    FSL_CHECK_NOTHROW(url("http://urquell-fn.appspot.com/lib/echo/*Afsk1YSP"));
    FSL_CHECK_NOTHROW(
            url("http://urquell-fn.appspot.com/lib/json/object/basic_data"));
    FSL_CHECK_NOTHROW(url(
            "http://urquell-fn.appspot.com/lib/json/object/basic_data?__="));
    FSL_CHECK_NOTHROW(url(
            "http://l.yimg.com/a/combo?"
            "arc/yui/reset_2.6.7.css&"
            "arc/yui/fonts_2.6.4.css&metro/uiplugins/generic_0.1.13.css&"
            "metro/error/error_0.1.18.css&metro/fp/fp_zindex_0.0.34.css&"
            "metro/fp/fp_0.1.101.css&metro/uiplugins/tablist_service_0.1.7.css&"
            "metro/uiplugins/iframeshim_service_0.0.5.css&"
            "metro/uiplugins/menu_service_0.1.4.css&"
            "metro/masthead/masthead_0.2.101.css&"
            "metro/navbar/navbar_0.1.119.css&"
            "metro/navbar/navbar_pageoptions_0.0.44.css&"
            "metro/sda/sda_0.1.37.css&"
            "metro/tuc/tuc_outboxlite_common_0.0.24.css&"
            "metro/tuc/tuc_outboxlite_embedded_0.0.12.css&"
            "metro/pa/pa_0.1.196.css&"
            "metro/pa/pa_detached_0.1.84.css&"
            "metro/uiplugins/tooltip_service_1.0.5.css&"
            "metro/uiplugins/tooltip_default_0.1.19.css&"
            "metro/pa/tooltip_pa_dialog_1.0.12.css&"
            "metro/uiplugins/sortable_service_0.1.9.css&"
            "metro/pa/pa_add_0.1.55.css&"
            "metro/uiplugins/tablist_news_0.0.16.css&"
            "metro2/simplenews/simplenews_0.1.19.css&"
            "metro/footer/footer_0.1.68.css&metro/footer/"
            "subfooter_0.0.11.css"));
    FSL_CHECK_NOTHROW(
            url("http://www.google.com/coop/cse/brand?"
                "form=cse-search-box%26%2312296%3B=th"));
    FSL_CHECK(not fostlib::url("http://localhost/").fragment().has_value());
    FSL_CHECK_EQ(fostlib::url("http://localhost/#").fragment(), "");
    FSL_CHECK_EQ(
            fostlib::url("http://localhost/#some%20thing").fragment(),
            "some%20thing");
}


FSL_TEST_FUNCTION(url_join) {
    using namespace f5::literals;
    url base("https://loc:45/some/path?query=yes#fragment");
    FSL_CHECK_EQ(
            url(base, "http://example.com").as_string(), "http://example.com/");
    FSL_CHECK_EQ(
            url(base, "https://example.com").as_string(),
            "https://example.com/");
    FSL_CHECK_EQ(
            url(base, "https://example.com:4567").as_string(),
            "https://example.com:4567/");
    FSL_CHECK_EQ(
            url(base, "://example.com/foo").as_string(),
            "https://example.com/foo");
    FSL_CHECK_EQ(url(base, "/new/path").as_string(), "https://loc:45/new/path");
    FSL_CHECK_EQ(
            url(base, "file.html").as_string(),
            "https://loc:45/some/file.html");
    FSL_CHECK_EQ(url(base, "./where").as_string(), "https://loc:45/some/where");
    FSL_CHECK_EQ(url(base, "../where").as_string(), "https://loc:45/where");
    FSL_CHECK_EQ(url(base, ".").as_string(), "https://loc:45/some/");
    FSL_CHECK_EQ(url(base, "?q").as_string(), "https://loc:45/some/path?q");
    FSL_CHECK_EQ(
            url(base, "?q#bd").as_string(), "https://loc:45/some/path?q#bd");
    FSL_CHECK_EQ(
            url(base, "#fr").as_string(),
            "https://loc:45/some/path?query=yes#fr");
    FSL_CHECK_EQ(
            url(base, "#").as_string(), "https://loc:45/some/path?query=yes#");
    /// **TODO** The below test needs to throw rather than
    FSL_CHECK_EQ(
            url(base, "../../where").as_string(), "https://loc:45/../where");
}


#define TEST_COERCION(u) FSL_CHECK_EQ(coerce<string>(url(u)), u);
FSL_TEST_FUNCTION(z_coercion) {
    TEST_COERCION("http://localhost/file-path.html");
    TEST_COERCION("http://localhost/~somebody");
    TEST_COERCION("http://localhost:8000/~somebody");
    TEST_COERCION("http://bmf.miro.felspar.net/extranet/login/?next=/office/");
    TEST_COERCION(
            "http://bmf.miro.felspar.net:8000/extranet/login/?next=/office/");
}
