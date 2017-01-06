/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/counter>
#include <fost/rask/rask-proto.hpp>


namespace fostlib {


    /// A set of counters that are used for protocols and versions etc.
    struct rask_counters {
        rask_counters(const fostlib::module &base);

        fostlib::performance cnx_created;
    };


}

