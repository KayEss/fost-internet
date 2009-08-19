/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/http.useragent.hpp>
#include <fost/parse/parse.hpp>

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

    utf8string first_line;
    *cnx >> first_line;
    string protocol, message; int status;
    if ( !boost::spirit::parse(first_line.c_str(),
        (
            boost::spirit::strlit< wliteral >(L"HTTP/0.9") |
            boost::spirit::strlit< wliteral >(L"HTTP/1.0") |
            boost::spirit::strlit< wliteral >(L"HTTP/1.1")
        )[ phoenix::var(protocol) = phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 ) ]
        >> boost::spirit::chlit< wchar_t >( ' ' )
        >> boost::spirit::uint_parser< int, 10, 3, 3 >()[ phoenix::var(status) = phoenix::arg1 ]
        >> boost::spirit::chlit< wchar_t >( ' ' )
        >> (
            +boost::spirit::chset<>( L"a-zA-Z " )
        )[ phoenix::var(message) = phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 ) ]
    ).full )
        throw exceptions::not_implemented("Expected a HTTP response", first_line);

    return std::auto_ptr< http::user_agent::response >(new http::user_agent::response(
        cnx, req.method(), req.address(),
        protocol, status, message
    ));
}


/*
    fostlib::http::user_agent::request
*/


fostlib::http::user_agent::request::request(const string &method, const url &url)
: text_body(string()), method(method), address(url) {
}
fostlib::http::user_agent::request::request(const string &method, const url &url, const string &data)
: text_body(data), method(method), address(url) {
}


/*
    fostlib::http::user_agent::response
*/

fostlib::http::user_agent::response::response(
    std::auto_ptr< network_connection > connection,
    const string &method, const url &url,
    const string &protocol, int status, const string &message
) : method(method), address(url), protocol(protocol), status(status), message(message), m_cnx(connection) {
    while ( true ) {
        utf8string line;
        *m_cnx >> line;
        if (line.empty())
            break;
        headers().parse(coerce< string >(line));
    }
    content_type(headers()[ L"Content-Type" ].value());
}


const mime &fostlib::http::user_agent::response::body() const {
    if ( !m_body ) {
        nullable< int64_t > length;
        if ( method() == L"HEAD" )
            length = 0;
        else if (headers().exists("Content-Length"))
            length = coerce< int64_t >(headers()["Content-Length"].value());

        if ( content_type().substr(0, 5) == "text/" ) {
            const nullable< string > charset( headers()["Content-Type"].subvalue("charset") );
            if ( charset.isnull() || charset == "utf-8" || charset == "UTF-8" ) {
                try {
                    if ( !length.isnull() ) {
                        std::vector< utf8 > body_text(length.value());
                        *m_cnx >> body_text;
                        m_body.reset(new text_body(&body_text[0], &body_text[0] + length.value()));
                    } else {
                        boost::asio::streambuf body_buffer;
                        *m_cnx >> body_buffer;
                        utf8string body_text;
                        body_text.reserve(body_buffer.size());
                        while ( body_buffer.size() )
                            body_text += body_buffer.sbumpc();
                        m_body.reset(new text_body(body_text));
                    }
                } catch ( fostlib::exceptions::exception &e ) {
                    if ( charset.isnull() )
                        e.info() << L"Assumed that the page was UTF-8 as the charset from the Content-Type header was blank\n";
                    else
                        e.info() << L"Charset in Content-Type header given as " << charset.value() << L"\n";
                    throw;
                }
            } else
                throw exceptions::not_implemented("fostlib::http::user_agent::response::body() -- where the encoding is not UTF-8", charset.value());
        } else
            throw exceptions::not_implemented("fostlib::http::user_agent::response::body() -- where the content is not text");
    }
    return *m_body;
}
