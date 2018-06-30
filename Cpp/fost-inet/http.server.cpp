/**
    Copyright 2008-2018, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include "fost-inet.hpp"
#include <fost/http.server.hpp>
#include <fost/parse/http.server.hpp>

#include <fost/exception/parse_error.hpp>
#include <fost/log>
#include <fost/threading>


using namespace fostlib;


/**
    ## fostlib::http::server
*/


fostlib::http::server::server( const host &h, uint16_t p )
: binding( h ), port( p ), m_server(
    m_service, boost::asio::ip::tcp::endpoint( binding().address(), port() )
) {
}

std::unique_ptr< http::server::request > fostlib::http::server::operator () () {
    std::unique_ptr< boost::asio::io_service > io_service(new boost::asio::io_service);
    auto sock = std::make_unique<boost::asio::ip::tcp::socket>(*io_service);
    m_server.accept(*sock);
    return std::unique_ptr< http::server::request >(
        new http::server::request(std::move(io_service), std::move(sock)));
}

namespace {
    bool service(
        std::function< bool ( http::server::request & ) > service_lambda,
        boost::asio::io_service *servicep,
        boost::asio::ip::tcp::socket *sockp
    ) {
        std::unique_ptr< boost::asio::io_service > io_service(servicep);
        std::unique_ptr< boost::asio::ip::tcp::socket > usockp(sockp);
        try {
            http::server::request req(std::move(io_service), std::move(usockp));
            try {
                return service_lambda(req);
            } catch ( fostlib::exceptions::exception &e ) {
                auto estr = fostlib::coerce<fostlib::string>(e);
                log::error(c_fost_inet)
                    ("", "web server service -- exception caught")
                    ("exception", estr);
                text_body error(estr);
                req(error, 500);
                return true;
            }
        } catch ( fostlib::exceptions::exception & ) {
            // A 400 response has already been sent by the request handler
            return true;
        }
    }

    void respond_on_socket(
        fostlib::network_connection *cnx,
        mime &response, const ascii_string &status
    ) {
        std::stringstream buffer;
        response.headers().fold_limit(null); // Turn off MIME line folding
        buffer << "HTTP/1.0 " << status.underlying() << "\r\n"
            << response.headers() << "\r\n";
        *cnx << buffer;
        for ( mime::const_iterator i( response.begin() ); i != response.end(); ++i ) {
            *cnx << *i;
        }
    }

    void raise_connection_error (
        const mime &response, const ascii_string &status
    ) {
        throw exceptions::null(
            "This is a mock server request. It cannot send a response to any client");
    }

    bool return_false() {
        return false;
    }

}

void fostlib::http::server::operator () (
    std::function< bool (http::server::request &) > service_lambda
) {
    (*this)(service_lambda, return_false);
}

void fostlib::http::server::operator () (
    std::function< bool (http::server::request &) > service_lambda,
    std::function< bool (void) > terminate_lambda
) {
    // Create a worker pool to service the requests
    workerpool pool;
    while ( true ) {
        // Use a raw pointer here for minimum overhead -- if it all goes wrong
        // and a socket leaks, we don't care (for now)
        boost::asio::io_service *service(new boost::asio::io_service);
        boost::asio::ip::tcp::socket *sock(
            new boost::asio::ip::tcp::socket(*service));
        m_server.accept(*sock);
        if ( terminate_lambda() ) {
            delete sock;
            return;
        }
        pool.f<bool>([service_lambda, service, sock]() {
            return ::service(service_lambda, service, sock);
        });
    }
}


/**
    ## fostlib::http::server::request
*/


fostlib::http::server::request::request()
: m_method("GET"), m_pathspec("/"), m_mime(new binary_body()) {
}


fostlib::http::server::request::request(
        std::unique_ptr<boost::asio::io_service> io_service,
        std::unique_ptr<boost::asio::ip::tcp::socket> connection)
: m_cnx(new network_connection(std::move(io_service), std::move(connection))),
        m_handler(raise_connection_error) {
    m_handler = [this](mime &m, const ascii_string &h) {
        respond_on_socket(m_cnx.get(), m, h);
    };

    utf8_string first_line;
    (*m_cnx) >> first_line;

    try {
        {
            request_status parsed_line;
            auto pos = first_line.begin(), end = first_line.end();
            server_first_line<utf8_string::const_iterator> rule;
            if ( not boost::spirit::qi::parse(pos, end, rule, parsed_line) || pos != end ) {
                log::error(c_fost_inet)
                    ("message", "First line failed to parse")
                    ("first line", coerce<string>(first_line))
                    ("unparsed", string(pos, end));
                throw exceptions::parse_error("Expected a HTTP request", coerce<string>(first_line));
            }
            m_method = parsed_line.method;
            m_pathspec = parsed_line.path;
            m_query_string = parsed_line.query;
        }

        mime::mime_headers headers;
        while ( true ) {
            utf8_string line;
            *m_cnx >> line;
            if ( line.empty() )
                break;
            headers.parse(coerce< string >(line));
        }

        headers.add("__remote_addr", m_cnx->remote_end().name());

        std::size_t content_length = 0;
        if ( headers.exists("Content-Length") )
            content_length = coerce< int64_t >(headers["Content-Length"].value());

        if ( content_length ) {
            std::vector< unsigned char > data( content_length );
            *m_cnx >> data;
            m_mime.reset( new binary_body(data, headers) );
        } else {
            m_mime.reset( new binary_body(headers) );
        }
    } catch ( fostlib::exceptions::exception &e ) {
        try {
            text_body error(coerce<string>(e));
            (*this)(error, 400);
        } catch ( ... ) {
            log::warning(c_fost_inet,
                "Exception whilst sending bad request response");
            absorb_exception();
        }
        throw;
    }
}


fostlib::http::server::request::request(
    const string &method,
    const url::filepath_string &filespec,
    std::unique_ptr< binary_body > headers_and_body,
    const url::query_string &qs
) : m_handler(raise_connection_error),
        m_method( method ), m_pathspec( filespec ), m_query_string(qs),
        m_mime( headers_and_body.get()
            ? headers_and_body.release()
            : new binary_body() ) {
    m_mime->headers().add("__remote_addr", "127.0.0.1");
}


fostlib::http::server::request::request(
    const string &method,
    const url::filepath_string &filespec,
    const url::query_string &qs,
    std::unique_ptr< binary_body > headers_and_body
) : m_handler(raise_connection_error),
        m_method( method ), m_pathspec( filespec ), m_query_string(qs),
        m_mime( headers_and_body.get()
            ? headers_and_body.release()
            : new binary_body() ) {
    m_mime->headers().add("__remote_addr", "127.0.0.1");
}


fostlib::http::server::request::request(
    const string &method, const url::filepath_string &filespec,
    std::unique_ptr< binary_body > headers_and_body,
    std::function<void (const mime&, const ascii_string&)> handler
) : m_handler(handler),
        m_method( method ), m_pathspec( filespec ),
        m_mime( headers_and_body.release() ) {
    m_mime->headers().add("__remote_addr", "127.0.0.1");
}


fostlib::host fostlib::http::server::request::remote_address() const {
    static const setting<string> header("fost-internet/Cpp/fost-inet/http.server.cpp",
        "webserver", "Remote address header", "__remote_addr", true);
    return host(m_mime->headers()[header.value()].value());
}


fostlib::nullable<fostlib::json>
    fostlib::http::server::request::operator [] (const jcursor &pos) const
{
    if ( pos.size() ) {
        const auto size = pos.size();
        if ( pos[0] == "headers" && (size == 2 || size == 3) ) {
            auto header = boost::get<string>(pos[1]);
            if ( headers().exists(header) ) {
                if ( size == 2 ) {
                    return json(headers()[header].value());
                } else {
                    auto subvalue = headers()[header].subvalue(boost::get<string>(pos[2]));
                    if ( not subvalue ) return null;
                    else return json(subvalue.value());
                }
            } else {
                return null;
            }
        }
    }
    throw exceptions::not_implemented(__func__,
        "Requested path into request is not possible",
        fostlib::coerce<fostlib::json>(pos));
}


boost::shared_ptr< fostlib::binary_body > fostlib::http::server::request::data(
) const {
    if ( !m_mime.get() )
        throw exceptions::null(
            "This server request has no MIME data, not even headers");
    return m_mime;
}


void fostlib::http::server::request::operator() (
    mime &response, const ascii_string &status
) {
    m_handler(response, status);
}


nliteral fostlib::http::server::status_text( int code ) {
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
        case 423: return "Locked";
        case 449: return "Retry With";

        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 506: return "HTTP Version Not Supported";

        default: return "(unknown status code)";
    }
}


void fostlib::http::server::request::operator() (
    mime &response, const int status
) {
    std::stringstream ss;
    ss << status << " " << status_text(status);
    (*this)(response, ss.str());
}

