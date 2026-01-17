#pragma once


#include <fost/counter>
#include <fost/hod/hod.hpp>


namespace fostlib {
    namespace hod {


        /// A set of counters that are used for protocols and versions etc.
        struct counters {
            counters(const fostlib::module &base);

            fostlib::performance cnx_created;
        };


    }
}
