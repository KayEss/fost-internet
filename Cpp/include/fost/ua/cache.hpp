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
    json get_json(url const &, headers const & = headers{});


    /// ## Testing support

    /// Set an expectation for a `GET` request
    void expect_get(url const &, json, headers const & = headers{});


}
