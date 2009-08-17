/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <boost/asio/ssl.hpp>
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

std::auto_ptr< http::user_agent::response > fostlib::http::user_agent::operator () (
    const string &method, const url &url, const nullable< string > &data
) {
    std::auto_ptr< network_connection > cnx;
    if ( url.protocol() == ascii_string("http") )
        cnx = std::auto_ptr< network_connection >(
            new network_connection(url.server(), url.port())
        );
    else if ( url.protocol() == ascii_string("https") ) {
        boost::asio::ssl::context ctx(g_io_service, boost::asio::ssl::context::sslv23_client);
        ctx.set_verify_mode(boost::asio::ssl::context::verify_none);

        boost::asio::ip::tcp::socket socket(g_io_service);
        socket.connect(boost::asio::ip::tcp::endpoint(url.server().address(), url.port()));

        boost::asio::ssl::stream< boost::asio::ip::tcp::socket& > ssl_sock(socket, ctx);
        ssl_sock.handshake(boost::asio::ssl::stream_base::client);

        throw exceptions::not_implemented( L"HTTPS not supported" );
    } else
        throw exceptions::not_implemented( L"fostlib::http::user_agent with this protocol", coerce< string >(url.protocol()) );

    std::stringstream buffer;
    buffer << coerce< utf8string >( method ) << " " << url.pathspec().underlying().underlying() << " HTTP/1.1\r\n";
    text_body request(data.value(string()));
    request.headers().add("Host", url.server().name());
    request.print_on(buffer);
    *cnx << buffer;

    if ( !data.isnull() ) {
        utf8string utf8data = coerce< utf8string >(data.value());
        *cnx << utf8data;
    }

    return std::auto_ptr< http::user_agent::response >(new http::user_agent::response(cnx, method, url));
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
