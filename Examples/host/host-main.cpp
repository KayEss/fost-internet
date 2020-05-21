/**
    Copyright 2008-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/internet>
#include <fost/cli>
#include <fost/main.hpp>


FSL_MAIN("host", "host\n" "Copyright (C) 2008-2020 Red Anchor Trading Co. Ltd.")
(fostlib::ostream &out, fostlib::arguments &args) {
    for (fostlib::arguments::size_type c(1); c < args.size(); ++c) try {
            out << args[c].value() << " -> "
                << fostlib::coerce<fostlib::string>(fostlib::host{args[c].value()}.address()) << std::endl;
        } catch (fostlib::exceptions::host_not_found &) {
            out << args[c].value() << " -- Address not found" << std::endl;
        }
    return 0;
}
