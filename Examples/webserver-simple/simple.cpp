#include <fost/cli>
#include <fost/main.hpp>
#include <fost/internet>
#include <fost/http.server.hpp>


namespace {
    fostlib::setting<fostlib::string>
            c_host("http-simple", "Server", "Bind to", "localhost");
    fostlib::setting<int> c_port("http-simple", "Server", "Port", 8001);
}


FSL_MAIN("http-simple", "Simple HTTP server")
(fostlib::ostream &o, fostlib::arguments &args) {
    fostlib::http::server server(
            fostlib::host(args[1].value_or(c_host.value())), c_port.value());
    o << "Answering requests on http://" << server.binding() << ":"
      << server.port() << "/" << std::endl;
    for (bool process(true); process;) {
        std::unique_ptr<fostlib::http::server::request> req(server());
        o << req->method() << " " << req->file_spec() << std::endl;
        fostlib::text_body response{
                "<html><body>This <b>is</b> a response</body></html>",
                {},
                "text/html"};
        (*req)(response);
        if (req->data()->headers()["Host"].value() == "localhost") {
            process = false;
        }
    }
    o << "Told to exit\n";
    return 0;
}
