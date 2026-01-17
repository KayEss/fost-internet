#include <fost/counter>

#include <fost/hod/connection.hpp>


namespace {
    fostlib::performance
            p_cnx_created(fostlib::hod::c_hod, "connections", "created");
}


fostlib::hod::connection_base::connection_base()
: peer_version{}, id(++p_cnx_created) {}
