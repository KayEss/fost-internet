#pragma once


#include <fost/counter>


namespace fostlib::ua {


    /// The number oF HTTP requests that have been made. Requests
    /// involving redirects will count multiple times.
    extern fostlib::performance p_network_requests;

    /// The number of redirects that have been followed
    extern fostlib::performance p_redirects_followed;

    /// The number of error pages returned. This counts incorrect statuses
    /// only, not networking problems.
    extern fostlib::performance p_status_errors;


}
