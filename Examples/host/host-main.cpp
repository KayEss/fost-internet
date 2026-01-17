#include <fost/internet>
#include <fost/cli>
#include <fost/main.hpp>


FSL_MAIN(
        "host",
        "host")
(fostlib::ostream &out, fostlib::arguments &args) {
    for (fostlib::arguments::size_type c(1); c < args.size(); ++c) try {
            out << args[c].value() << " -> "
                << fostlib::coerce<fostlib::string>(
                           fostlib::host{args[c].value()}.address())
                << std::endl;
        } catch (fostlib::exceptions::host_not_found &) {
            out << args[c].value() << " -- Address not found" << std::endl;
        }
    return 0;
}
