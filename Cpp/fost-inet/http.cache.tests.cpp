/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/ua/cache>
#include <fost/ua/cache.detail.hpp>
#include <fost/ua/exceptions.hpp>
#include <fost/test>


FSL_TEST_SUITE(http_cache);


FSL_TEST_FUNCTION(expectations) {
    fostlib::ua::ua_test test;

    fostlib::url const u1;
    fostlib::ua::expect_get(u1, fostlib::json{});
    FSL_CHECK_EQ(fostlib::ua::get_json(u1), fostlib::json{});
    FSL_CHECK_EQ(fostlib::ua::get_json(u1), fostlib::json{});

    fostlib::ua::expect_post(u1, fostlib::json{});
    FSL_CHECK_EQ(fostlib::ua::post_json(u1, fostlib::json{}), fostlib::json{});
    FSL_CHECK_EXCEPTION(
            fostlib::ua::post_json(u1, fostlib::json{}),
            fostlib::ua::no_expectation &);

    fostlib::url const u2{u1, "/foo"};
    fostlib::ua::expect_get(u2, fostlib::json{"fred"});
    fostlib::ua::expect_get(u2, fostlib::json{"barney"});
    FSL_CHECK_EQ(fostlib::ua::get_json(u2), fostlib::json{"fred"});
    FSL_CHECK_EQ(fostlib::ua::get_json(u2), fostlib::json{"barney"});
    FSL_CHECK_EQ(fostlib::ua::get_json(u2), fostlib::json{"barney"});
    FSL_CHECK_EQ(fostlib::ua::get_json(u1), fostlib::json{});

    fostlib::ua::expect_post(u2, fostlib::json{"foo"});
    FSL_CHECK_EQ(
            fostlib::ua::post_json(u2, fostlib::json{}), fostlib::json{"foo"});
    FSL_CHECK_EXCEPTION(
            fostlib::ua::post_json(u2, fostlib::json{}),
            fostlib::ua::no_expectation &);

    fostlib::ua::expect_get(u1, fostlib::json{"bar"});
    /// If we don't track properly that this GET result has already
    /// been returned then the expectation above will not result
    /// in the correct return value for the first of the requests
    /// below.
    FSL_CHECK_EQ(fostlib::ua::get_json(u1), fostlib::json{"bar"});
    FSL_CHECK_EQ(fostlib::ua::get_json(u1), fostlib::json{"bar"});

    fostlib::ua::expect_get(u2, fostlib::json{1});
    fostlib::ua::expect_get(u2, fostlib::json{2});
    FSL_CHECK_EQ(fostlib::ua::get_json(u2), fostlib::json{1});
    FSL_CHECK_EQ(fostlib::ua::get_json(u2), fostlib::json{2});
    FSL_CHECK_EQ(fostlib::ua::get_json(u2), fostlib::json{2});

    fostlib::ua::expect_post(u1, fostlib::json{true});
    fostlib::ua::expect_post(u1, fostlib::json{false});
    FSL_CHECK_EQ(
            fostlib::ua::post_json(u1, fostlib::json{}), fostlib::json{true});
    FSL_CHECK_EQ(
            fostlib::ua::post_json(u1, fostlib::json{}), fostlib::json{false});
    FSL_CHECK_EXCEPTION(
            fostlib::ua::post_json(u1, fostlib::json{}),
            fostlib::ua::no_expectation &);

    fostlib::ua::expect_get(u2, fostlib::json{"fred"});
    fostlib::ua::expect_get(u2, fostlib::json{"barney"});
    fostlib::ua::expect_post(u1, fostlib::json{true});
    fostlib::ua::expect_post(u1, fostlib::json{false});
    FSL_CHECK_EQ(
            fostlib::ua::post_json(u1, fostlib::json{}), fostlib::json{true});
    FSL_CHECK_EQ(fostlib::ua::get_json(u2), fostlib::json{"fred"});
    FSL_CHECK_EQ(
            fostlib::ua::post_json(u1, fostlib::json{}), fostlib::json{false});
    FSL_CHECK_EXCEPTION(
            fostlib::ua::post_json(u1, fostlib::json{}),
            fostlib::ua::no_expectation &);
    FSL_CHECK_EQ(fostlib::ua::get_json(u2), fostlib::json{"barney"});
    FSL_CHECK_EQ(fostlib::ua::get_json(u2), fostlib::json{"barney"});

    fostlib::ua::clear_expectations();
    FSL_CHECK_EXCEPTION(
            fostlib::ua::get_json(u1), fostlib::ua::no_expectation &);
    FSL_CHECK_EXCEPTION(
            fostlib::ua::post_json(u1, fostlib::json{}),
            fostlib::ua::no_expectation &);
}


FSL_TEST_FUNCTION(login) {
    fostlib::url const profile{"https://example.com/me"},
            login{"https://example.com/login"};

    /// We want to test something like this function for fetching profile data
    auto const do_login = [&]() {
        /// A real implementation would take a username/password
        fostlib::ua::headers heads;
        auto bearer = fostlib::ua::post_json(
                login, fostlib::json{"username/password"});
        heads.set(
                "Authorization",
                "bearer=" + fostlib::coerce<fostlib::string>(bearer));
        return heads;
    };
    auto const get_profile = [&](auto heads) {
        try {
            return fostlib::ua::get_json(profile, heads);
        } catch (fostlib::ua::unauthorized &) {
            auto authed = do_login();
            return fostlib::ua::get_json(profile, authed);
        }
    };

    /// This would be the test function we would want to write
    fostlib::ua::ua_test test;

    fostlib::ua::headers wrong, right;
    wrong.set("Authorization", "bearer=FOO");
    right.set("Authorization", "bearer=BAR");

    fostlib::ua::expect(
            "GET", profile, fostlib::ua::unauthorized{profile}, wrong);
    fostlib::ua::expect_post(login, fostlib::json{"BAR"});
    fostlib::ua::expect_get(profile, fostlib::json{"Profile data"}, right);

    /// Now get the profile data
    FSL_CHECK_EQ(get_profile(wrong), "Profile data");
}


/// ## Implementation details


FSL_TEST_FUNCTION(cache_keys) {
    using fostlib::url;
    fostlib::ua::headers const headers;
    FSL_CHECK_EQ(
            fostlib::ua::cache_key("GET", url{"http://localhost/"}, headers),
            "964dyfcyk487v6wzdsajacmxkj0zrgc6mk1bdmprd3vzjvd83h90");

    /// Changing the HTTP method must result in a different cache
    /// key.
    FSL_CHECK_EQ(
            fostlib::ua::cache_key("POST", url{"http://localhost/"}, headers),
            "wfh0cz13mw8k49rh5fcjntcqjezbpsv4ghsfs4n90cajm2rhvy90");

    /// The following change to the URL must all result in different
    /// cache keys.
    FSL_CHECK_EQ(
            fostlib::ua::cache_key("GET", url{"https://localhost/"}, headers),
            "9y1kh50530hdb09p0erzpwht99xy529n63v529m164n8xr9nvrng");
    FSL_CHECK_EQ(
            fostlib::ua::cache_key(
                    "GET", url{"http://localhost:8080/"}, headers),
            "r1k901351tk4zb3w6bz2dfd0edg433v7e2e69crfp6dfxhqecf50");
    FSL_CHECK_EQ(
            fostlib::ua::cache_key("GET", url{"http://localhost/foo"}, headers),
            "6j8eybwpe5s1sh9d5eyg9by6edfa1eqe8rpxk57z8bezfb4nsqag");
    FSL_CHECK_EQ(
            fostlib::ua::cache_key(
                    "GET", url{"http://localhost/?foo=bar"}, headers),
            "qw4cs8ygp1raksja7sps5m626jsynpkjkcg3ravxnkswe8f0rh2g");

    /// The `Authorization` must result in a different hash key
    auto authorization = headers;
    authorization.add("Authorization", "bearer=FOO");
    FSL_CHECK_EQ(
            fostlib::ua::cache_key(
                    "GET", url{"http://localhost/"}, authorization),
            "hc16ja53bhwz2c2dns53eb1gb6qsmn4cthqadm14tncfhsa9a3v0");

    /// The following changes must not result in different cache keys
    FSL_CHECK_EQ(
            fostlib::ua::cache_key("GET", url{"http://localhost/#foo"}, headers),
            "964dyfcyk487v6wzdsajacmxkj0zrgc6mk1bdmprd3vzjvd83h90");
    FSL_CHECK_EQ(
            fostlib::ua::cache_key("GET", url{"http://localhost"}, headers),
            "964dyfcyk487v6wzdsajacmxkj0zrgc6mk1bdmprd3vzjvd83h90");

    /// The following should be a unique hash key, but because of the
    /// default host port it is not
    FSL_CHECK_EQ(
            fostlib::ua::cache_key("GET", url{"http://localhost:80/"}, headers),
            "964dyfcyk487v6wzdsajacmxkj0zrgc6mk1bdmprd3vzjvd83h90");
}


FSL_TEST_FUNCTION(method_idempotency) {
    FSL_CHECK(fostlib::ua::idempotent("GET"));
    FSL_CHECK(not fostlib::ua::idempotent("POST"));
}
