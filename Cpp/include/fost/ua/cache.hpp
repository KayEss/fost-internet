/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/mime.hpp>
#include <fost/url.hpp>


namespace fostlib::ua {


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
            f5::u8view method, url const &, json body, headers const &);
    inline json get_json(url const &u, headers const &h = headers{}) {
        return request_json("GET", u, json{}, h);
    }
    inline json post_json(
            url const &u, fostlib::json body, headers const &h = headers{}) {
        return request_json("POST", u, std::move(body), h);
    }


    /// ## Testing support

    /// Clear all expectations. Should be used at the start of a test
    void clear_expectations();

    /// Set an expectation for a request
    void expect(
            f5::u8view method, url const &, json, headers const & = headers{});
    inline void expect_get(url const &u, json b, headers const &h = headers{}) {
        expect("GET", u, b, h);
    }
    inline void expect_post(url const &u, json b, headers const &h = headers{}) {
        expect("POST", u, b, h);
    }


}
