/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/counter>


namespace fostlib::ua {


    /// The number oF HTTP requests that have been made. Requests
    /// involving redirects will count multiple times.
    extern fostlib::performance p_network_requests;

    /// The number of error pages returned. This counts incorrect statuses
    /// only, not networking problems.
    extern fostlib::performance p_status_errors;


}
