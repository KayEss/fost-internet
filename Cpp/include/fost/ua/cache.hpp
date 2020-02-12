/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/mime.hpp>
#include <fost/url.hpp>


namespace fostlib::ua {


    extern module const c_fost_inet_ua;


    /// Type alias to shorten a name
    using headers = mime::mime_headers;


    /// ## Configuration

    /**
     * The folder used for the persistent cache. If this is not set
     * then no caching will be performed and all requests will
     * go upstream.
     */
    extern fostlib::setting<std::optional<fostlib::string>> const c_cache_folder;

    /**
     * If this is set to `true` then no actual HTTP/HTTPS requests
     * will be made, even in the case of an expectation or cache
     * miss.
     *
     * If testing code this should normally be set `true` with
     * the `c_cache_folder` left as `null` so that any request made
     * without an expectation doesn't result in an actual
     * network request.
     */
    extern fostlib::setting<bool> const c_force_no_http_requests;

    /// ## User agent functions

    /// Fetch the JSON body of the requested URL
    json request_json(
            f5::u8view method, url const &, std::optional<json> body, headers);
    inline json get_json(url const &u, headers h = headers{}) {
        return request_json("GET", u, {}, std::move(h));
    }
    inline json
            post_json(url const &u, fostlib::json body, headers h = headers{}) {
        return request_json("POST", u, std::move(body), std::move(h));
    }


    /// ## Testing support

    /// Instantiate an instance of this type at the start of a test that
    /// involves the user agent to clear previous expectations and
    /// make sure that HTTP requests are not actually made
    struct ua_test {
        ua_test();

        decltype(c_cache_folder) const no_cache{"test", c_cache_folder, {}};
        decltype(c_force_no_http_requests)
                const no_http{"test", c_force_no_http_requests, true};
    };

    /// Clear all expectations.
    void clear_expectations();

    /// Set an expectation for a request
    void expect(
            f5::u8view method, url const &, json, headers const & = headers{});
    /// Set an expectation that request processing will throw
    void
            expect(f5::u8view method,
                   url const &,
                   std::exception_ptr expected,
                   headers const & = headers{});
    template<typename E>
    inline std::enable_if_t<std::is_base_of_v<std::exception, E>>
            expect(f5::u8view method,
                   url const &url,
                   E &&expected,
                   headers const &h = headers{}) {
        try {
            throw expected;
        } catch (std::exception &) {
            expect(method, url, std::current_exception(), h);
        }
    }

    /// Wrappers for common APIs
    inline void expect_get(url const &u, json b, headers const &h = headers{}) {
        expect("GET", u, b, h);
    }
    inline void expect_post(url const &u, json b, headers const &h = headers{}) {
        expect("POST", u, b, h);
    }


}
