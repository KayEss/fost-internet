/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/connection.hpp>

#include <fost/exception/not_implemented.hpp>
#include <fost/exception/unexpected_eof.hpp>


using namespace fostlib;


namespace {
    boost::asio::io_service g_io_service;
}


fostlib::network_connection::network_connection(std::auto_ptr< boost::asio::ip::tcp::socket > socket)
: m_socket(socket), m_ssl_data(NULL) {
}

fostlib::network_connection::network_connection(const host &h, nullable< port_number > p)
: m_socket(new boost::asio::ip::tcp::socket(g_io_service)), m_ssl_data(NULL) {
    m_socket->connect(boost::asio::ip::tcp::endpoint(
        h.address(), p.value(coerce< port_number >(h.service().value("0")))
    ));
}


network_connection &fostlib::network_connection::operator << ( const utf8string &s ) {
    boost::asio::streambuf b;
    std::ostream os(&b);
    os << s;
    std::size_t length(m_socket->send(b.data()));
    b.consume(length);
    return *this;
}
network_connection &fostlib::network_connection::operator << ( const std::stringstream &ss ) {
    return this->operator << ( ss.str() );
}


network_connection &fostlib::network_connection::operator >> ( utf8string &s ) {
    std::size_t length(boost::asio::read_until(*m_socket, m_input_buffer, "\r\n"));
    if ( length >= 2 ) {
        for ( std::size_t c = 0; c < length - 2; ++c )
            s += m_input_buffer.sbumpc();
        m_input_buffer.sbumpc(); m_input_buffer.sbumpc();
    } else
        throw fostlib::exceptions::unexpected_eof("Could not find a \\r\\n sequence before network connection ended");
    return *this;
}
#include <iostream>
network_connection &fostlib::network_connection::operator >> ( std::vector< utf8 > &v ) {
    try {
        std::size_t length(boost::asio::read(*m_socket, m_input_buffer, boost::asio::transfer_at_least(v.size() - m_input_buffer.size())));
        if ( m_input_buffer.size() != v.size() )
            throw fostlib::exceptions::unexpected_eof("Could not read all of the requested bytes from the network connection");
        for ( std::size_t p = 0; p <  v.size(); ++p )
            v[p] = m_input_buffer.sbumpc();
        return *this;
    } catch ( std::exception &e ) {
        std::cout << "Whilst reading " << v.size() << " bytes" << std::endl;
        throw;
    }
}

