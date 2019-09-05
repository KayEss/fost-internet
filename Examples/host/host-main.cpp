/**
    Copyright 2008-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/internet>
#include <fost/cli>
#include <fost/main.hpp>


using namespace fostlib;


FSL_MAIN(L"host", L"host\nCopyright (C) 2008-2009 Felspar Co. Ltd.")
(ostream &out, arguments &args) {
    for (arguments::size_type c(1); c < args.size(); ++c) try {
            out << args[c].value() << L" -> "
                << coerce<string>(host(args[c].value()).address()) << std::endl;
        } catch (exceptions::host_not_found &) {
            out << args[c].value() << L" -- Address not found" << std::endl;
        }
    return 0;
}
