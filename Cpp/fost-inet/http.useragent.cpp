/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/http.useragent.hpp>

#include <fost/exception/not_implemented.hpp>
#include <fost/exception/unexpected_eof.hpp>


using namespace fostlib;


/*
    fostlib::http::user_agent
*/

namespace {
    boost::asio::io_service g_io_service;
}


fostlib::http::user_agent::user_agent() {
}
fostlib::http::user_agent::user_agent(const url &u)
: base(u) {
}


std::auto_ptr< http::user_agent::response > fostlib::http::user_agent::operator () (request &req) {
    req.headers().add("Host", req.address().server().name());

    if ( !authentication().isnull() )
        authentication().value()( req );

    std::stringstream buffer;
    buffer << coerce< utf8string >( req.method() ) << " " << req.address().pathspec().underlying().underlying() << " HTTP/1.1\r\n";
    req.print_on(buffer);

    std::auto_ptr< network_connection > cnx(
        new network_connection(req.address().server(), req.address().port())
    );
    if ( req.address().protocol() == ascii_string("https") )
        cnx->start_ssl();

    *cnx << buffer;

    if ( !req.text().empty() )
        *cnx << req.text();

    return std::auto_ptr< http::user_agent::response >(new http::user_agent::response(cnx, req.method(), req.address()));
}


/*
    fostlib::http::user_agent::request
*/


fostlib::http::user_agent::request::request(const string &method, const url &url, const nullable< string > &data)
: text_body(data.value(string())), method(method), address(url) {
}


/*
    fostlib::http::user_agent::response
*/

fostlib::http::user_agent::response::response(
    std::auto_ptr< network_connection > connection,
    const string &method, const url &url
) : method(method), location(url), m_cnx(connection) {
    utf8string first_line;

    *m_cnx >> first_line;

    while ( true ) {
        utf8string line;
        *m_cnx >> line;
        if (line.empty())
            break;
        headers().parse(coerce< string >(line));
    }
    content_type(headers()[ L"Content-Type" ].value());
}

#include <fost/exception/not_implemented.hpp>
std::auto_ptr< mime > fostlib::http::user_agent::response::body() {
    if (!headers().exists("Content-Length"))
        throw exceptions::not_implemented("fostlib::http::user_agent::response::body() -- where there is no content length");
    if ( content_type().substr(0, 5) == "text/" ) {
        int64_t length = coerce< int64_t >(headers()["Content-Length"].value());

        const nullable< string > charset( headers()["Content-Type"].subvalue("charset") );
        if ( charset.isnull() || charset == "utf-8" || charset == "UTF-8" ) {
            std::vector< utf8 > body_text(length);
            *m_cnx >> body_text;
            try {
                return std::auto_ptr< mime >(new text_body(&body_text[0], &body_text[0] + length));
            } catch ( fostlib::exceptions::exception &e ) {
                if ( charset.isnull() )
                    e.info() << L"Assumed that the page was UTF-8 as the charset from the Content-Type header was blank\n";
                else
                    e.info() << L"Charset in Content-Type header given as " << charset.value() << L"\n";
                throw;
            }
        } else
            throw exceptions::not_implemented("fostlib::http::user_agent::response::body() -- where the encoding is not UTF-8");
    } else
        throw exceptions::not_implemented("fostlib::http::user_agent::response::body() -- where the content is not text");
}
