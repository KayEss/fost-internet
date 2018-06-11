/**
    Copyright 2017-2018, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once


#include <fost/counter>
#include <fost/rask/hod.hpp>


namespace fostlib { namespace hod {


    /// A set of counters that are used for protocols and versions etc.
    struct counters {
        counters(const fostlib::module &base);

        fostlib::performance cnx_created;
    };


}}

