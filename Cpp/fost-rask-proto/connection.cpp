/*
    Copyright 2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/counter>

#include <fost/rask/rask-proto.hpp>
#include <fost/rask/tcp.hpp>


namespace {
    const fostlib::module c_tcp(fostlib::c_rask_proto, "tcp");
    fostlib::performance p_cnx_created(c_tcp, "connections", "created");
}


fostlib::rask_tcp::rask_tcp()
: id(++p_cnx_created) {
}

