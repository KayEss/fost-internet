/*
    Copyright 2008-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/datetime>

#include <fost/http.useragent.hpp>
#include <fost/parse/parse.hpp>

#include <fost/exception/unexpected_eof.hpp>


using namespace fostlib;


/*
    fostlib::http::user_agent
*/


namespace {
    boost::asio::io_service g_io_service;

    const fostlib::setting< fostlib::string > c_user_agent(
        L"fost-internet/Cpp/fost-inet/http.useragent.cpp",
        L"HTTP", L"UserAgent", L"Felspar user agent", true
    );
}


fostlib::http::user_agent::user_agent() {
}
fostlib::http::user_agent::user_agent(const url &u)
: base(u) {
}


std::auto_ptr< http::user_agent::response > fostlib::http::user_agent::operator () (request &req) const {
    req.headers().set("Date", coerce< string >(
        coerce< rfc1123_timestamp >(timestamp::now())
    ));
    req.headers().set("Host", req.address().server().name());
    if ( !req.headers().exists("User-Agent") )
        req.headers().set("User-Agent", c_user_agent.value() + L"/Fost 4");
    req.headers().set("TE", "trailers");

    if ( !authentication().isnull() )
        authentication().value()( req );

    std::auto_ptr< network_connection > cnx(
        new network_connection(req.address().server(), req.address().port())
    );
    if ( req.address().protocol() == ascii_printable_string("https") )
        cnx->start_ssl();

    std::stringstream buffer;
    buffer << coerce< utf8_string >( req.method() ).underlying() << " " <<
        req.address().pathspec().underlying().underlying();
    {
        nullable< ascii_printable_string > q = req.address().query().as_string();
        if ( !q.isnull() )
            buffer << "?" << q.value().underlying();
    }
    buffer << " HTTP/1.1\r\n" << req.headers() << "\r\n";
    *cnx << buffer;

    for ( mime::const_iterator i( req.data().begin() ); i != req.data().end(); ++i )
        *cnx << *i;

    utf8_string first_line;
    *cnx >> first_line;
    string protocol, message; int status;
    if ( !boost::spirit::parse(first_line.underlying().c_str(),
        (
            boost::spirit::strlit< wliteral >(L"HTTP/0.9") |
            boost::spirit::strlit< wliteral >(L"HTTP/1.0") |
            boost::spirit::strlit< wliteral >(L"HTTP/1.1")
        )[ phoenix::var(protocol) =
            phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 ) ]
        >> boost::spirit::chlit< wchar_t >( ' ' )
        >> boost::spirit::uint_parser< int, 10, 3, 3 >()
            [ phoenix::var(status) = phoenix::arg1 ]
        >> boost::spirit::chlit< wchar_t >( ' ' )
        >> (
            +boost::spirit::chset<>( L"a-zA-Z " )
        )[ phoenix::var(message) =
            phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 ) ]
    ).full )
        throw exceptions::not_implemented(
            "Expected a HTTP response", coerce< string >(first_line)
        );

    return std::auto_ptr< http::user_agent::response >(
        new http::user_agent::response(
            cnx, req.method(), req.address(),
            protocol, status, message
        )
    );
}


/*
    fostlib::http::user_agent::request
*/


fostlib::http::user_agent::request::request(const string &method, const url &url)
: m_data(new empty_mime), method(method), address(url) {
}
fostlib::http::user_agent::request::request(
    const string &method, const url &url, const string &data
) : m_data(new text_body(data)), method(method), address(url) {
}
fostlib::http::user_agent::request::request(
    const string &method, const url &url, const boost::filesystem::wpath &data
) : m_data(new file_body(data)), method(method), address(url) {
}
fostlib::http::user_agent::request::request(
    const string &method, const url &url, boost::shared_ptr< mime > mime_data
) : m_data(mime_data), method(method), address(url) {
}


/*
    fostlib::http::user_agent::response
*/


namespace {
    void read_headers(
        network_connection &cnx, mime::mime_headers &headers
    ) {
        while ( true ) {
            utf8_string line;
            cnx >> line;
            if (line.empty())
                break;
            headers.parse(coerce< string >(line));
        }
    }
}


fostlib::http::user_agent::response::response(
    std::auto_ptr< network_connection > connection,
    const string &method, const url &url,
    const string &protocol, int status, const string &message
) : method(method), address(url), protocol(protocol),
status(status), message(message), m_cnx(connection) {
    read_headers(*m_cnx, m_headers);
}


const binary_body &fostlib::http::user_agent::response::body() {
    if ( !m_body ) {
        nullable< int64_t > length;
        if ( method() == L"HEAD" )
            length = 0;
        else if (m_headers.exists("Content-Length"))
            length = coerce< int64_t >(m_headers["Content-Length"].value());

        if ( !length.isnull() && length.value() == 0 )
            m_body.reset(new binary_body(m_headers));
        else if ( length.isnull() ) {
            if ( m_headers["Transfer-Encoding"].value() == "chunked" ) {
                std::vector< unsigned char > data;
                while ( true ) {
                    std::string length, ignore;
                    *m_cnx >> length;
                    std::size_t chunk_size = fostlib::coerce< std::size_t >(
                        hex_string(length)
                    );
                    if ( chunk_size == 0 )
                        break;
                    std::vector< unsigned char > chunk( chunk_size );
                    *m_cnx >> chunk >> ignore;
                    data.insert(data.end(), chunk.begin(), chunk.end());
                }
                // Read trailing headers
                read_headers(*m_cnx, m_headers);
                m_body.reset(new binary_body(data, m_headers));
            } else {
                boost::asio::streambuf body_buffer;
                *m_cnx >> body_buffer;
                std::vector< unsigned char > body_data;
                body_data.reserve(body_buffer.size());
                while ( body_buffer.size() )
                    body_data.push_back( body_buffer.sbumpc() );
                m_body.reset(new binary_body(body_data, m_headers));
            }
        } else {
            std::vector< unsigned char > body(length.value());
            *m_cnx >> body;
            m_body.reset(new binary_body(body, m_headers));
        }
    }
    return *m_body;
}
