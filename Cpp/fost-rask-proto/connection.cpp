/*
    Copyright 2016-2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/counter>

#include <fost/rask/rask-proto.hpp>
#include <fost/rask/tcp.hpp>


namespace {
    fostlib::performance p_cnx_created(rask::c_rask_proto, "connections", "created");
}


rask::connection_base::connection_base()
: id(++p_cnx_created) {
}

