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

    const fostlib::setting< fostlib::string > c_user_agent( L"fost-internet/Cpp/fost-inet/http.useragent.cpp", L"HTTP", L"UserAgent", L"Felspar user agent", true );
}


fostlib::http::user_agent::user_agent() {
}
fostlib::http::user_agent::user_agent(const url &u)
: base(u) {
}


std::auto_ptr< http::user_agent::response > fostlib::http::user_agent::operator () (request &req) const {
    req.headers().add("Host", req.address().server().name());
    if ( !req.headers().exists("User-Agent") )
        req.headers().add("User-Agent", c_user_agent.value() + L"/Fost 4.09.09");

    if ( !authentication().isnull() )
        authentication().value()( req );

    std::stringstream buffer;
    buffer << coerce< utf8string >( req.method() ) << " " << req.address().pathspec().underlying().underlying() << " HTTP/1.0\r\n";
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


std::auto_ptr< mime > fostlib::http::user_agent::response::body() {
    nullable< int64_t > length;
    if (headers().exists("Content-Length"))
        length = coerce< int64_t >(headers()["Content-Length"].value());

    if ( content_type().substr(0, 5) == "text/" ) {
        const nullable< string > charset( headers()["Content-Type"].subvalue("charset") );
        if ( charset.isnull() || charset == "utf-8" || charset == "UTF-8" ) {
            try {
                if ( !length.isnull() ) {
                    std::vector< utf8 > body_text(length.value());
                    *m_cnx >> body_text;
                    return std::auto_ptr< mime >(new text_body(&body_text[0], &body_text[0] + length.value()));
                } else {
                    boost::asio::streambuf body_buffer;
                    *m_cnx >> body_buffer;
                    utf8string body_text;
                    body_text.reserve(body_buffer.size());
                    while ( body_buffer.size() )
                        body_text += body_buffer.sbumpc();
                    return std::auto_ptr< mime >(new text_body(body_text));
                }
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
