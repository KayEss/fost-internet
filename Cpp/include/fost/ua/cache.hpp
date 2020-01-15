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


    /// ## User agent functions

    /// Fetch the JSON body of the requested URL
    json get_json(url const &, headers const & = headers{});


    /// ## Testing support

    /// Set an expectation for a `GET` request
    void expect_get(url const &, json, headers const & = headers{});


}
