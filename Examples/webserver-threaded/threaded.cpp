/**
    Copyright 2008-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/main>
#include <fost/internet>
#include <fost/http.server.hpp>
#include <fost/threading>


using namespace fostlib;


namespace {
    setting<string>
            c_host(L"http-threaded", L"Server", L"Bind to", L"localhost");
    setting<int> c_port(L"http-threaded", L"Server", L"Port", 8001);

    bool service(http::server::request &req) {
        text_body response(
                L"<html><body>This <b>is</b> a response</body></html>",
                mime::mime_headers(), L"text/html");
        req(response);
        return true;
    }
}


FSL_MAIN(
        L"http-threaded",
        L"Threaded HTTP server\nCopyright (c) 2009-2016, Felspar Co. Ltd.")
(fostlib::ostream &o, fostlib::arguments &args) {
    // Bind server to host and port
    http::server server(host(args[1].value_or(c_host.value())), c_port.value());
    o << L"Answering requests on http://" << server.binding() << L":"
      << server.port() << L"/" << std::endl;
    // Service requests
    server(service);
    // It will never get this far
    return 0;
}
