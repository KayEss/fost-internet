#include <fost/hod/counters.hpp>


fostlib::hod::counters::counters(const fostlib::module &base)
: cnx_created(base, "created") {}
