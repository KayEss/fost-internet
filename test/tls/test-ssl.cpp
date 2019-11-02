/**
    Copyright 2008-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/cli>
#include <fost/main>
#include <fost/http>


namespace {
    const fostlib::setting<std::optional<fostlib::string>> c_output{
            __FILE__, "test-ssl", "Output file", fostlib::null, true};
    const fostlib::setting<bool> c_works{
            __FILE__, "test-ssl", "Valid TLS and certificates", true, true};

    void worked() {
        if (c_output.value()) {
            fostlib::utf::save_file(
                    fostlib::coerce<fostlib::fs::path>(c_output.value().value()),
                    "");
        }
    }
}


FSL_MAIN(
        "test-ssl",
        "Check HTTPS connections and failures\n"
        "Copyright (c) 2008-2019 Red Anchor Trading Co. Ltd.")
(fostlib::ostream &o, fostlib::arguments &args) {
    args.commandSwitch("o", c_output);
    args.commandSwitch("w", c_works);

    // The URL to be fetched (default to localhost)
    fostlib::string location = args[1].value_or("http://badssl.com/");
    // Create a user agent and request the URL
    try {
        fostlib::http::user_agent browser;
        fostlib::http::user_agent::request request(
                "GET", fostlib::url{location});
        auto response(browser(request));
        if (c_works.value()) {
            worked();
            return 0;
        } else {
            return 1;
        }
    } catch (fostlib::exceptions::exception &e) {
        if (c_works.value()) {
            return 2;
        } else {
            worked();
            return 0;
        }
    }
}
