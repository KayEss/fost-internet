/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/http.server.hpp>
#include <fost/thread.hpp>
#include <fost/parse/url.hpp>


using namespace fostlib;


/*
    fostlib::http::server
*/


fostlib::http::server::server( const host &h, uint16_t p )
: binding( h ), port( p ), m_server( m_service, boost::asio::ip::tcp::endpoint( binding().address(), port() ) ) {
}

std::auto_ptr< http::server::request > fostlib::http::server::operator() () {
    std::auto_ptr< boost::asio::ip::tcp::socket > sock( new boost::asio::ip::tcp::socket( m_service ) );
    m_server.accept( *sock );
    return std::auto_ptr< http::server::request >( new http::server::request( sock ) );
}


/*
    fostlib::http::server::request
*/


fostlib::http::server::request::request( std::auto_ptr< boost::asio::ip::tcp::socket > socket )
: m_cnx( socket ) {
    utf8string first_line;
    m_cnx >> first_line;
    if ( !boost::spirit::parse(first_line.c_str(),
        (
            boost::spirit::strlit< wliteral >(L"GET")
        )[ phoenix::var(m_method) = phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 ) ]
        >> boost::spirit::chlit< wchar_t >( ' ' )
        >> url_filespec_p[ phoenix::var(m_pathspec) = phoenix::construct_< url::filepath_string >( phoenix::arg1 ) ]
        >> !(
            boost::spirit::chlit< wchar_t >( ' ' )
            >> (
                boost::spirit::strlit< wliteral >(L"HTTP/1.0") |
                boost::spirit::strlit< wliteral >(L"HTTP/1.1")
            )
        )
    ).full )
        throw exceptions::not_implemented("Expected a HTTP request", coerce< string >(first_line));

    mime::mime_headers headers;
    while ( true ) {
        utf8string line;
        m_cnx >> line;
        if ( line.empty() )
            break;
        headers.parse(line);
    }

    if ( method() == L"GET" )
        m_mime.reset( new empty_mime(headers) );
    else
        throw exceptions::not_implemented(L"HTTP method " + method(), coerce< string >(first_line));
}


const mime &fostlib::http::server::request::data() const {
    if ( !m_mime.get() )
        throw exceptions::null("This server request has no MIME data, not even headers");
    return *m_mime;
}


void fostlib::http::server::request::operator() ( const mime &response ) {
    std::stringstream buffer;
    buffer << "HTTP/1.0 200 OK\r\n" << response.headers() << "\r\n";
    m_cnx << buffer;
    for ( mime::const_iterator i( response.begin() ); i != response.end(); ++i )
        m_cnx << *i;
}
