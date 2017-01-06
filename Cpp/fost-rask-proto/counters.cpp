/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/rask/counters.hpp>


fostlib::rask_counters::rask_counters(const fostlib::module &base)
: cnx_created(base, "created") {
}

