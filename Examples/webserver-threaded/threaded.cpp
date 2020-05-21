/**
    Copyright 2008-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/main>
#include <fost/internet>
#include <fost/http.server.hpp>
#include <fost/threading>


namespace {
    fostlib::setting<fostlib::string>
            c_host("http-threaded", "Server", "Bind to", "localhost");
    fostlib::setting<int> c_port("http-threaded", "Server", "Port", 8001);

    bool service(fostlib::http::server::request &req) {
        fostlib::text_body response(
                "<html><body>This <b>is</b> a response</body></html>",
                fostlib::mime::mime_headers(), "text/html");
        req(response);
        return true;
    }
}


FSL_MAIN(
        "http-threaded",
        "Threaded HTTP server\n"
        "Copyright (c) 2009-2020 Red Anchor Trading Co. Ltd.")
(fostlib::ostream &o, fostlib::arguments &args) {
    // Bind server to host and port
    fostlib::http::server server(
            fostlib::host{args[1].value_or(c_host.value())}, c_port.value());
    o << "Answering requests on http://" << server.binding() << ":"
      << server.port() << "/" << std::endl;
    // Service requests
    server(service);
    // It will never get this far
    return 0;
}
