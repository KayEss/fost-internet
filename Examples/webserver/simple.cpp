/*
    Copyright 2008-2009 Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/cli>
#include <fost/main.hpp>
#include <fost/internet>
#include <fost/detail/http.server.hpp>


using namespace fostlib;


namespace {
    setting< string > c_host( L"http-simple", L"Server", L"Bind to", L"localhost" );
    setting< int > c_port( L"http-simple", L"Server", L"Port", 8001 );
}


FSL_MAIN(
    L"http-simple",
    L"Simple HTTP server\nCopyright (c) 2008-2009, Felspar Co. Ltd."
)( fostlib::ostream &o, fostlib::arguments & ) {
    http::server server( host( c_host.value() ), c_port.value() );
    o << L"Answering requests on http://" << server.binding() << L":" << server.port() << L"/" << std::endl;
    for ( bool process( true ); process; ) {
        std::auto_ptr< http::server::request > req( server() );
        o << req->method() << L" " << req->file_spec() << std::endl;
        (*req)( text_body( L"<html><body>This <b>is</b> a response</body></html>", mime::mime_headers(), L"text/html" ) );
        process = false;
    };
    o << L"Told to exit" << std::endl;
    return 0;
}
