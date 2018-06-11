/**
    Copyright 2017-2018, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <fost/hod/counters.hpp>


fostlib::hod::counters::counters(const fostlib::module &base)
: cnx_created(base, "created") {
}

