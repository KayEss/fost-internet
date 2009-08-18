/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/connection.hpp>
#include <boost/asio/ssl.hpp>

#include <fost/exception/not_implemented.hpp>
#include <fost/exception/unexpected_eof.hpp>


using namespace fostlib;


namespace {
    boost::asio::io_service g_io_service;
}


struct ssl_data {
    ssl_data(boost::asio::ip::tcp::socket &sock)
    : ctx(g_io_service, boost::asio::ssl::context::sslv23_client),
    ssl_sock(sock, ctx) {
        ssl_sock.handshake(boost::asio::ssl::stream_base::client);
    }

    boost::asio::ssl::context ctx;
    boost::asio::ssl::stream< boost::asio::ip::tcp::socket& > ssl_sock;
};
struct network_connection::ssl : public ssl_data {
    ssl(boost::asio::ip::tcp::socket &sock)
    : ssl_data(sock) {
    }
};


fostlib::network_connection::network_connection(std::auto_ptr< boost::asio::ip::tcp::socket > socket)
: m_socket(socket), m_ssl_data(NULL) {
}

fostlib::network_connection::network_connection(const host &h, nullable< port_number > p)
: m_socket(new boost::asio::ip::tcp::socket(g_io_service)), m_ssl_data(NULL) {
    m_socket->connect(boost::asio::ip::tcp::endpoint(
        h.address(), p.value(coerce< port_number >(h.service().value("0")))
    ));
}

fostlib::network_connection::~network_connection() {
    delete m_ssl_data;
}


void fostlib::network_connection::start_ssl() {
    m_ssl_data = new ssl(*m_socket);
}
namespace {
    std::size_t send(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, boost::asio::streambuf &b) {
        if ( ssl )
            return boost::asio::write(ssl->ssl_sock, b);
        else
            return sock.send(b.data());
    }
    std::size_t read_until(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, boost::asio::streambuf &b, const char *term) {
        if ( ssl )
            return boost::asio::read_until(ssl->ssl_sock, b, term);
        else
            return boost::asio::read_until(sock, b, term);
    }
    template< typename F >
    std::size_t read(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, boost::asio::streambuf &b, F f) {
        if ( ssl )
            return boost::asio::read(ssl->ssl_sock, b, f);
        else
            return boost::asio::read(sock, b, f);
    }
    template< typename F >
    std::size_t read(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, boost::asio::streambuf &b, F f, boost::system::error_code &e) {
        if ( ssl )
            return boost::asio::read(ssl->ssl_sock, b, f, e);
        else
            return boost::asio::read(sock, b, f, e);
    }
}


network_connection &fostlib::network_connection::operator << ( const utf8string &s ) {
    boost::asio::streambuf b;
    std::ostream os(&b);
    os << s;
    std::size_t length(send(*m_socket, m_ssl_data, b));
    b.consume(length);
    return *this;
}
network_connection &fostlib::network_connection::operator << ( const std::stringstream &ss ) {
    return this->operator << ( ss.str() );
}


network_connection &fostlib::network_connection::operator >> ( utf8string &s ) {
    std::size_t length(read_until(*m_socket, m_ssl_data, m_input_buffer, "\r\n"));
    if ( length >= 2 ) {
        for ( std::size_t c = 0; c < length - 2; ++c )
            s += m_input_buffer.sbumpc();
        m_input_buffer.sbumpc(); m_input_buffer.sbumpc();
    } else
        throw fostlib::exceptions::unexpected_eof("Could not find a \\r\\n sequence before network connection ended");
    return *this;
}
network_connection &fostlib::network_connection::operator >> ( std::vector< utf8 > &v ) {
    read(*m_socket, m_ssl_data, m_input_buffer, boost::asio::transfer_at_least(v.size() - m_input_buffer.size()));
    if ( m_input_buffer.size() != v.size() )
        throw fostlib::exceptions::unexpected_eof("Could not read all of the requested bytes from the network connection");
    for ( std::size_t p = 0; p <  v.size(); ++p )
        v[p] = m_input_buffer.sbumpc();
    return *this;
}
void fostlib::network_connection::operator >> ( boost::asio::streambuf &b ) {
    boost::system::error_code error;
    read(*m_socket, m_ssl_data, m_input_buffer, boost::asio::transfer_all(), error);
    if ( error != boost::asio::error::eof )
        throw boost::system::system_error(error);
    while ( m_input_buffer.size() )
        b.sputc(m_input_buffer.sbumpc());
}
