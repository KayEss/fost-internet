/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/http.server.hpp>
#include <fost/thread.hpp>

#include <fost/exception/not_implemented.hpp>


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
: m_cnx( new network_connection(socket) ) {
}
const string &fostlib::http::server::request::method() {
    throw exceptions::not_implemented("fostlib::http::server::request::method()");
}
const string &fostlib::http::server::request::file_spec() {
    throw exceptions::not_implemented("fostlib::http::server::request::file_spec()");
}

void fostlib::http::server::request::operator() ( const mime &response ) {
    throw exceptions::not_implemented("fostlib::http::server::request::operator() ( const mime &response )");
}
