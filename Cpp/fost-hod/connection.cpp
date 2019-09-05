/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/counter>

#include <fost/hod/connection.hpp>


namespace {
    fostlib::performance
            p_cnx_created(fostlib::hod::c_hod, "connections", "created");
}


fostlib::hod::connection_base::connection_base()
: peer_version{}, id(++p_cnx_created) {}
