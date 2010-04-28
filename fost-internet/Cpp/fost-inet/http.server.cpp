/*
    Copyright 2008-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/threading>
#include <fost/http.server.hpp>
#include <fost/parse/url.hpp>


using namespace fostlib;


/*
    fostlib::http::server
*/


fostlib::http::server::server( const host &h, uint16_t p )
: binding( h ), port( p ), m_server(
    m_service, boost::asio::ip::tcp::endpoint( binding().address(), port() )
) {
}

std::auto_ptr< http::server::request > fostlib::http::server::operator () () {
    std::auto_ptr< boost::asio::ip::tcp::socket > sock(
        new boost::asio::ip::tcp::socket( m_service )
    );
    m_server.accept( *sock );
    return std::auto_ptr< http::server::request >(
        new http::server::request( sock )
    );
}

namespace {
    bool service(
        boost::function< bool ( http::server::request & ) > service_lambda,
        boost::asio::ip::tcp::socket *sockp
    ) {
        std::auto_ptr< boost::asio::ip::tcp::socket > sock(sockp);
        http::server::request req;
        try {
            req(sock);
        } catch ( fostlib::exceptions::exception &e ) {
            text_body error(
                fostlib::coerce<fostlib::string>(e)
            );
            req( error, 400 );
            return false;
        }
        try {
            return service_lambda(req);
        } catch ( fostlib::exceptions::exception &e ) {
            text_body error(
                fostlib::coerce<fostlib::string>(e)
            );
            req( error, 500 );
            return false;
        }
    }
}
void fostlib::http::server::operator () (
    boost::function< bool ( http::server::request & ) > service_lambda
) {
    // Create a worker pool to service the requests
    workerpool pool;
    while ( true ) {
        // Use a raw pointer here for minimum overhead -- if it all goes wrong
        // and a socket leaks, we don't care (for now)
        boost::asio::ip::tcp::socket *sock(
            new boost::asio::ip::tcp::socket( m_service )
        );
        m_server.accept( *sock );
        pool.f<bool>( boost::lambda::bind(service, service_lambda, sock) );
    }
}


/*
    fostlib::http::server::request
*/


fostlib::http::server::request::request() {
}
fostlib::http::server::request::request(
    std::auto_ptr< boost::asio::ip::tcp::socket > connection
) {
    (*this)(connection);
}
fostlib::http::server::request::request(
    const string &method, const url::filepath_string &filespec,
    std::auto_ptr< binary_body > headers_and_body
) : m_method( method ), m_pathspec( filespec ),
        m_mime( headers_and_body.release() ) {
}


namespace {
    /*
        This is really nasty, but we have to do it as Boost.Spirit is not
        thread safe.
    */
    boost::mutex g_parser_mutex;
}
void fostlib::http::server::request::operator () (
    std::auto_ptr< boost::asio::ip::tcp::socket > socket
) {
    m_cnx.reset( new network_connection(socket) );
    utf8_string first_line;
    (*m_cnx) >> first_line;
    {
        boost::mutex::scoped_lock lock(g_parser_mutex);
        if ( !boost::spirit::parse(first_line.underlying().c_str(),
            (
                +boost::spirit::chset<>( "A-Z" )
            )[
                phoenix::var(m_method) =
                    phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 )
            ]
            >> boost::spirit::chlit< char >( ' ' )
            >> (+boost::spirit::chset<>( "_a-zA-Z0-9/.,:()%=-" ))[
                phoenix::var(m_pathspec) =
                    phoenix::construct_< url::filepath_string >(
                        phoenix::arg1, phoenix::arg2
                    )
            ]
            >> !(
                boost::spirit::chlit< char >('?')
                >> (+boost::spirit::chset<>( "&\\/:_@a-zA-Z0-9.,%+*=-" ))[
                    phoenix::var(m_query_string) =
                        phoenix::construct_< ascii_printable_string >(
                            phoenix::arg1, phoenix::arg2
                        )
                ]
            )
            >> !(
                boost::spirit::chlit< char >( ' ' )
                >> (
                    boost::spirit::strlit< nliteral >("HTTP/1.0") |
                    boost::spirit::strlit< nliteral >("HTTP/1.1")
                )
            )
        ).full )
            throw exceptions::not_implemented(
                "Expected a HTTP request", coerce< string >(first_line)
            );
    }

    mime::mime_headers headers;
    while ( true ) {
        utf8_string line;
        *m_cnx >> line;
        if ( line.empty() )
            break;
        headers.parse(coerce< string >(line));
    }

    std::size_t content_length = 0;
    if ( headers.exists("Content-Length") )
        content_length = coerce< int64_t >(headers["Content-Length"].value());

    if ( content_length ) {
        std::vector< unsigned char > data( content_length );
        *m_cnx >> data;
        m_mime.reset( new binary_body(data, headers) );
    } else
        m_mime.reset( new binary_body(headers) );
}


boost::shared_ptr< fostlib::binary_body > fostlib::http::server::request::data(
) const {
    if ( !m_mime.get() )
        throw exceptions::null(
            "This server request has no MIME data, not even headers"
        );
    return m_mime;
}


void fostlib::http::server::request::operator() (
    const mime &response, const ascii_string &status
) {
    if ( !m_cnx.get() )
        throw exceptions::null(
            "This is a mock server request. It cannot send a response to any client"
        );
    std::stringstream buffer;
    buffer << "HTTP/1.0 " << status.underlying() << "\r\n"
        << response.headers() << "\r\n";
    (*m_cnx) << buffer;
    for ( mime::const_iterator i( response.begin() ); i != response.end(); ++i )
        (*m_cnx) << *i;
}


namespace {
    nliteral status_text( int code ) {
        switch (code) {
            case 100: return "Continue";
            case 101: return "Switching Protocols";

            case 200: return "OK";
            case 201: return "Created";
            case 202: return "Accepted";
            case 203: return "Non-Authoritative Information";
            case 204: return "No Content";
            case 205: return "Reset Content";
            case 206: return "Partial Content";
            case 207: return "Multi-Status";

            case 300: return "Multiple Choices";
            case 301: return "Moved Permanently";
            case 302: return "Found";
            case 303: return "See Other";
            case 304: return "Not Modified";
            case 305: return "Use Proxy";
            case 306: return "(Unused)";
            case 307: return "Temporary Redirect";

            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 402: return "Payment Required";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 406: return "Not Acceptable";
            case 407: return "Proxy Authentication Required";
            case 408: return "Request Timeout";
            case 409: return "Conflict";
            case 410: return "Gone";
            case 411: return "Length Required";
            case 412: return "Precondition Failed";
            case 413: return "Request Entity Too Large";
            case 414: return "Request-URI Too Long";
            case 415: return "Unsupported Media Type";
            case 416: return "Requested Range Not Satisfiable";
            case 417: return "Expectation Failed";

            case 500: return "Internal Server Error";
            case 501: return "Not Implemented";
            case 502: return "Bad Gateway";
            case 503: return "Service Unavailable";
            case 504: return "Gateway Timeout";
            case 506: return "HTTP Version Not Supported";

            default: return "(unknown status code)";
        }
    }
}
void fostlib::http::server::request::operator() (
    const mime &response, const int status
) {
    std::stringstream ss;
    ss << status << " " << status_text(status);
    (*this)(response, ss.str());
}
