/*
    Copyright 2008-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifdef FOST_OS_LINUX
    // Boost.ASIO checks a pointer for NULL which can never be NULL
    #pragma GCC diagnostic ignored "-Waddress"
#endif


#include "fost-inet.hpp"
#include <fost/connection.hpp>
#include <boost/asio/ssl.hpp>

#include <fost/exception/unexpected_eof.hpp>


using namespace fostlib;


namespace {
    boost::asio::io_service g_io_service;

    setting< json > c_socks_version("fost-internet/Cpp/fost-inet/connection.cpp",
        "Network settings", "Socks version", json(),
        true
    );
    setting< string > c_socks_host("fost-internet/Cpp/fost-inet/connection.cpp",
        "Network settings", "Socks host", L"localhost:8888",
        true
    );
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
namespace {

    void handle_error(
        nliteral func, nliteral msg,
        const boost::system::error_code &error
    ) {
        if ( error == boost::asio::error::eof )
            throw fostlib::exceptions::unexpected_eof(
                fostlib::string(msg)
            );
        else if ( error )
            throw fostlib::exceptions::not_implemented(func, error, msg);
    }

    std::size_t send(
        boost::asio::ip::tcp::socket &sock, ssl_data *ssl,
        boost::asio::streambuf &b
    ) {
        try {
            if ( ssl )
                return boost::asio::write(ssl->ssl_sock, b);
            else
                return sock.send(b.data());
        } catch ( boost::system::system_error &e ) {
            throw fostlib::exceptions::not_implemented(
                "send(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, boost::asio::streambuf &b)",
                e.code()
            );
        }
    }
    std::size_t read_until(
        boost::asio::ip::tcp::socket &sock, ssl_data *ssl,
        boost::asio::streambuf &b, const char *term
    ) {
        boost::system::error_code error;
        if ( ssl )
            return boost::asio::read_until(ssl->ssl_sock, b, term, error);
        else
            return boost::asio::read_until(sock, b, term, error);
        handle_error(
            "read_until(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, boost::asio::streambuf &b, const char *term)",
            "Whilst reading data from a socket", error
        );
    }
    template< typename F >
    std::size_t read(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, boost::asio::streambuf &b, F f) {
        boost::system::error_code error;
        if ( ssl )
            return boost::asio::read(ssl->ssl_sock, b, f, error);
        else
            return boost::asio::read(sock, b, f, error);
        handle_error(
            "read<F>(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, boost::asio::streambuf &b, F f)",
            "Whilst reading data from a socket", error
        );
    }
    template< typename F >
    std::size_t read(
        boost::asio::ip::tcp::socket &sock, ssl_data *ssl,
        boost::asio::streambuf &b, F f,
        boost::system::error_code &e
    ) {
        if ( ssl )
            return boost::asio::read(ssl->ssl_sock, b, f, e);
        else
            return boost::asio::read(sock, b, f, e);
    }

    void connect(
        boost::asio::ip::tcp::socket &socket, const host &host, port_number port
    ) {
        using namespace boost::asio::ip;
        tcp::resolver resolver(g_io_service);
        tcp::resolver::query q(
            coerce<ascii_string>(host.name()).underlying(),
            coerce<ascii_string>(coerce<string>(port)).underlying());
        boost::system::error_code host_error;
        tcp::resolver::iterator endpoint = resolver.resolve(q, host_error), end;
        if ( host_error == boost::asio::error::host_not_found )
            throw exceptions::host_not_found( host.name() );
        boost::system::error_code connect_error =
            boost::asio::error::host_not_found;
        while ( connect_error && endpoint != end ) {
            socket.close();
            socket.connect(*endpoint++, connect_error);
        }
        if ( connect_error )
            throw fostlib::exceptions::connect_failure(connect_error);
    }
}


fostlib::network_connection::network_connection(std::auto_ptr< boost::asio::ip::tcp::socket > socket)
: m_socket(socket), m_ssl_data(NULL) {
}

fostlib::network_connection::network_connection(const host &h, nullable< port_number > p)
: m_socket(new boost::asio::ip::tcp::socket(g_io_service)), m_ssl_data(NULL) {
    const port_number port = p.value(coerce< port_number >(h.service().value("0")));
    json socks(c_socks_version.value());

    if ( !socks.isnull() ) {
        const host socks_host( coerce< host >( c_socks_host.value() ) );
        connect(*m_socket, socks_host, coerce< port_number >(socks_host.service().value("0")));
        if ( c_socks_version.value() == json(4) ) {
            boost::asio::streambuf b;
            // Build and send the command to establish the connection
            b.sputc(0x4); // SOCKS v 4
            b.sputc(0x1); // stream
            b.sputc((port & 0xff00) >> 8); b.sputc(port & 0xff); // Destination port
            boost::asio::ip::address_v4::bytes_type bytes( h.address().to_v4().to_bytes() );
            for ( std::size_t p = 0; p < 4; ++p )
                b.sputc(bytes[p]);
            b.sputc(0); // User ID
            send(*m_socket, NULL, b);
            // Receive the response
            read(*m_socket, NULL, m_input_buffer, boost::asio::transfer_at_least(8));
            if ( m_input_buffer.sbumpc() != 0x00 || m_input_buffer.sbumpc() != 0x5a )
                throw exceptions::not_implemented("SOCKS 4 error handling where the response values are not 0x00 0x5a");
            char ignore[6];
            m_input_buffer.sgetn(ignore, 6);
        } else
            throw exceptions::not_implemented("SOCKS version not implemented", coerce< string >(c_socks_version.value()));
    } else
        connect(*m_socket, h, port);
}

fostlib::network_connection::~network_connection() {
    delete m_ssl_data;
}


void fostlib::network_connection::start_ssl() {
    m_ssl_data = new ssl(*m_socket);
}


network_connection &fostlib::network_connection::operator << ( const const_memory_block &p ) {
    const unsigned char
        *begin = reinterpret_cast< const unsigned char * >( p.first ),
        *end =  reinterpret_cast< const unsigned char * >( p.second )
    ;
    std::size_t length = end - begin;
    if ( length ) {
        boost::asio::streambuf b;
        for ( std::size_t pos = 0; pos != length; ++pos )
            b.sputc( begin[pos] );
        std::size_t sent(send(*m_socket, m_ssl_data, b));
        b.consume(sent);
    }
    return *this;
}
network_connection &fostlib::network_connection::operator << ( const utf8_string &s ) {
    boost::asio::streambuf b;
    std::ostream os(&b);
    os << s.underlying();
    std::size_t length(send(*m_socket, m_ssl_data, b));
    b.consume(length);
    return *this;
}
network_connection &fostlib::network_connection::operator << ( const std::stringstream &ss ) {
    return this->operator << ( ss.str() );
}


network_connection &fostlib::network_connection::operator >> ( utf8_string &s ) {
    std::string next;
    (*this) >> next;
    s += utf8_string(next);
    return *this;
}
network_connection &fostlib::network_connection::operator >> ( std::string &s ) {
    std::size_t length(read_until(*m_socket, m_ssl_data, m_input_buffer, "\r\n"));
    if ( length >= 2 ) {
        for ( std::size_t c = 0; c < length - 2; ++c )
            s += m_input_buffer.sbumpc();
        m_input_buffer.sbumpc(); m_input_buffer.sbumpc();
    } else
        throw fostlib::exceptions::unexpected_eof(
            "Could not find a \\r\\n sequence before network connection ended"
        );
    return *this;
}
network_connection &fostlib::network_connection::operator >> (
    std::vector< utf8 > &v
) {
    read(
        *m_socket, m_ssl_data, m_input_buffer,
        boost::asio::transfer_at_least(v.size() - m_input_buffer.size())
    );
    if ( m_input_buffer.size() < v.size() ) {
        fostlib::exceptions::unexpected_eof exception(
            "Could not read all of the requested bytes from the network connection"
        );
        exception.info()
            << "Read " << m_input_buffer.size()
            << " bytes out of " << v.size() << std::endl;
        throw exception;
    }
    for ( std::size_t p = 0; p <  v.size(); ++p )
        v[p] = m_input_buffer.sbumpc();
    return *this;
}
void fostlib::network_connection::operator >> ( boost::asio::streambuf &b ) {
    while ( m_input_buffer.size() )
        b.sputc(m_input_buffer.sbumpc());
    boost::system::error_code error;
    read(*m_socket, m_ssl_data, b, boost::asio::transfer_all(), error);
    if ( error != boost::asio::error::eof )
        throw fostlib::exceptions::not_implemented(
            "fostlib::network_connection::operator >> ( boost::asio::streambuf &b )",
            "Whilst reading into an Asio streambuf");
}


/*
    fostlib::exceptions::connect_failure
*/


fostlib::exceptions::connect_failure::connect_failure(
    boost::system::error_code error
) throw ()
: error(error) {
    info() << error << std::endl;
}


fostlib::wliteral const fostlib::exceptions::connect_failure::message()
        const throw () {
    return L"Network connection failure";
}
