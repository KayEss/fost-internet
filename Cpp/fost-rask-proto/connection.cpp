/**
    Copyright 2016-2018, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <fost/counter>

#include <fost/rask/rask-proto.hpp>
#include <fost/rask/tcp.hpp>


namespace {
    fostlib::performance p_cnx_created(fostlib::hod::c_rask_proto, "connections", "created");
}


fostlib::hod::connection_base::connection_base()
: peer_version{}, id(++p_cnx_created) {
}

