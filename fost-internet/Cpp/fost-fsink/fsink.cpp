/*
    Copyright 2010, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/main>
#include <fost/http>
#include <fost/http.server.hpp>


using namespace fostlib;


namespace {
    setting< string > c_host( L"fsink", L"Server", L"Bind to", L"0.0.0.0" );
    setting< int > c_port( L"fsink", L"Server", L"Port", 23456 );
}


FSL_MAIN(
    L"fsink",
    L"fsink - Fost logging sink\nCopyright (c) 1995-2010, Felspar Co. Ltd."
)( fostlib::ostream &out, fostlib::arguments &args ) {
    http::server server( host( args[1].value(c_host.value()) ), c_port.value() );
    out << L"Answering requests on "
        L"http://" << server.binding() << L":" << server.port() << L"/" << std::endl;
    for ( bool process( true ); process; ) {
        std::auto_ptr< http::server::request > req( server() );
        if ( req->method() == "PUT" ) {
            text_body response(L"Accepted\n", mime::mime_headers(), L"text/plain");
            out << *req->data() << std::endl;
            (*req)(response);
        } else {
            out << req->method() << L" " << req->file_spec() << std::endl;
            text_body response(L"Message?\n", mime::mime_headers(), "text/plain");
            (*req)(response);
        }
    }
    return 0;
}

