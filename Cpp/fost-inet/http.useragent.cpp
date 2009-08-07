/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/http.useragent.hpp>
#include <fost/exception/unexpected_eof.hpp>


using namespace fostlib;


/*
    fostlib::http::user_agent
*/

fostlib::http::user_agent::user_agent() {
}

std::auto_ptr< http::user_agent::response > fostlib::http::user_agent::operator () (
    const string &method, const url &url, const nullable< string > &data
) {
    std::auto_ptr< boost::asio::ip::tcp::iostream > cnx(
       new boost::asio::ip::tcp::iostream(boost::asio::ip::tcp::endpoint(url.server().address(), url.port().value(80)))
    );

    (*cnx) << coerce< utf8string >( method ) << " " << url.pathspec().underlying().underlying() << " HTTP/1.1\r\n";
    text_body request(data.value(string()));
    request.headers().add("Host", url.server().name());
    request.print_on(*cnx);

    if ( !data.isnull() ) {
        utf8string utf8data = coerce< utf8string >(data.value());
        cnx->write(utf8data.c_str(), utf8data.length());
    }

    cnx->flush();

    return std::auto_ptr< http::user_agent::response >(new http::user_agent::response(cnx, method, url));
}


/*
    fostlib::http::user_agent::response
*/

fostlib::http::user_agent::response::response(
    std::auto_ptr< boost::asio::ip::tcp::iostream > connection,
    const string &method, const url &url
) : method(method), location(url), m_stream(*connection), m_cnx(connection) {
    std::string http, status, message;

    m_stream >> http >> status;
    std::getline(m_stream, message, '\r');
    assert(m_stream.get() == '\n');

    while ( true ) {
        std::string line;
        std::getline(m_stream, line, '\r');
        assert(m_stream.get() == '\n');
        if (line.empty())
            break;
        headers().parse(line);
    }
    content_type(headers()[ L"Content-Type" ].value());
}

#include <fost/exception/not_implemented.hpp>
std::auto_ptr< mime > fostlib::http::user_agent::response::body() {
    if (!headers().exists("Content-Length"))
        throw exceptions::not_implemented("fostlib::http::user_agent::response::body() -- where there is no content length");

    int64_t length = coerce< int64_t >(headers()["Content-Length"].value());

    boost::scoped_array< utf8 > body_text( new utf8[ length ] );
    m_stream.read(reinterpret_cast< char* >(body_text.get()), length);
    if (m_stream.gcount() != length)
        throw exceptions::unexpected_eof("Not all request data was read");
    return std::auto_ptr< mime >(new text_body(body_text.get(), body_text.get() + length));
}
