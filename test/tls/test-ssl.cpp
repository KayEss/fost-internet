/**
    Copyright 2008-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/cli>
#include <fost/main>
#include <fost/http>


namespace {
    const fostlib::setting<std::optional<fostlib::string>>
            c_output(__FILE__, "test-ssl", "Output file", fostlib::null, true);
}


FSL_MAIN(
        "test-ssl",
        "Check HTTPS connections and failures\n"
        "Copyright (c) 2008-2019 Red Anchor Trading Co. Ltd.")
(fostlib::ostream &o, fostlib::arguments &args) {
    args.commandSwitch("o", c_output);

    // The URL to be fetched (default to localhost)
    fostlib::string location = args[1].value_or("http://badssl.com/");
    // Create a user agent and request the URL
    fostlib::http::user_agent browser;
    fostlib::http::user_agent::request request("GET", fostlib::url{location});
    auto response(browser(request));
    if (c_output.value()) {
        fostlib::utf::save_file(
                fostlib::coerce<fostlib::fs::path>(c_output.value().value()),
                "");
    }
    return 0;
}
