/*
    Copyright 2008-2011, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifdef FOST_OS_LINUX
    // Boost.ASIO checks a pointer for NULL which can never be NULL
    #pragma GCC diagnostic ignored "-Waddress"
#endif


#include "fost-inet.hpp"
#include <fost/insert>
#include <fost/connection.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/lambda/bind.hpp>


using namespace fostlib;


namespace {
    const setting< int64_t > c_connect_timeout(
        "fost-internet/Cpp/fost-inet/connection.cpp",
        "Network settings", "Connect time out", 10, true);
    const setting< int64_t > c_read_timeout(
        "fost-internet/Cpp/fost-inet/connection.cpp",
        "Network settings", "Read time out", 30, true);
    const setting< int64_t > c_large_read_chunk_size(
        "fost-internet/Cpp/fost-inet/connection.cpp",
        "Network settings", "Large read chunk size", 1024, true);
    const setting< json > c_socks_version(
        "fost-internet/Cpp/fost-inet/connection.cpp",
        "Network settings", "Socks version", json(), true);
    const setting< string > c_socks_host(
        "fost-internet/Cpp/fost-inet/connection.cpp",
        "Network settings", "Socks host", L"localhost:8888", true);
}


struct ssl_data {
    ssl_data(
        boost::asio::io_service &io_service, boost::asio::ip::tcp::socket &sock
    ) : ctx(io_service, boost::asio::ssl::context::sslv23_client),
    ssl_sock(sock, ctx) {
        ssl_sock.handshake(boost::asio::ssl::stream_base::client);
    }

    boost::asio::ssl::context ctx;
    boost::asio::ssl::stream< boost::asio::ip::tcp::socket& > ssl_sock;
};
struct network_connection::ssl : public ssl_data {
    ssl(boost::asio::io_service &io_service, boost::asio::ip::tcp::socket &sock)
    : ssl_data(io_service, sock) {
    }
};
namespace {

    void handle_error(
        nliteral func, nliteral msg,
        const boost::system::error_code &error
    ) {
        if ( error == boost::asio::error::eof )
            throw exceptions::unexpected_eof(string(msg));
        else if ( error )
            throw exceptions::not_implemented(func, error, msg);
    }

    std::size_t send(
        boost::asio::ip::tcp::socket &sock, ssl_data *ssl,
        boost::asio::streambuf &b
    ) {
        try {
            if ( ssl )
                return boost::asio::write(ssl->ssl_sock, b);
            else
                return boost::asio::write(sock, b);
        } catch ( boost::system::system_error &e ) {
            throw fostlib::exceptions::not_implemented(
                "send(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, "
                    "boost::asio::streambuf &b)",
                e.code()
            );
        }
    }

    struct timeout_wrapper {
        typedef nullable< boost::system::error_code > timeout_error;
        typedef nullable< std::pair< boost::system::error_code, std::size_t > >
            read_error;
        typedef boost::function<
            void (boost::system::error_code)
        > connect_async_function_type;
        typedef boost::function<
            void (boost::system::error_code, std::size_t)
        > read_async_function_type;

        boost::asio::ip::tcp::socket &sock;
        boost::system::error_code &error;
        boost::asio::deadline_timer timer;
        timeout_error timeout_result;
        read_error read_result;
        std::size_t received;

        static void timedout(
            boost::asio::ip::tcp::socket &sock,
            timeout_error &n, boost::system::error_code e
        ) {
            if ( e != boost::asio::error::operation_aborted ) {
#ifdef FOST_OS_WINDOWS
                sock.close();
#else
                boost::system::error_code cancel_error; // We ignore this
                sock.cancel(cancel_error);
#endif // FOST_OS_WINDOWS
            }
            n = e;
        }
        static void connect_done(
            boost::asio::deadline_timer &timer, read_error &n,
            boost::system::error_code e
        ) {
            timer.cancel();
            n = std::make_pair(e, 0);
        }
        static void read_done(
            boost::asio::deadline_timer &timer, read_error &n,
            boost::system::error_code e, std::size_t s
        ) {
            timer.cancel();
            n = std::make_pair(e, s);
        }

        timeout_wrapper(
            boost::asio::ip::tcp::socket &sock, boost::system::error_code &e,
            const setting< int64_t > &timeout = c_read_timeout
        ) : sock(sock), error(e), timer(sock.get_io_service()), received(0) {
            timer.expires_from_now(boost::posix_time::seconds(
                timeout.value()));
            timer.async_wait(boost::lambda::bind(&timedout,
                boost::ref(sock), boost::ref(timeout_result),
                boost::lambda::_1));
        }

        connect_async_function_type connect_async_function() {
            return boost::lambda::bind(&connect_done,
                boost::ref(timer), boost::ref(read_result),
                boost::lambda::_1);
        }
        read_async_function_type read_async_function() {
            return boost::lambda::bind(&read_done,
                boost::ref(timer), boost::ref(read_result),
                boost::lambda::_1, boost::lambda::_2);
        }

        std::size_t complete() {
            sock.get_io_service().reset();
            sock.get_io_service().run();
            if ( read_result.value().first &&
                    read_result.value().first != boost::asio::error::eof )
                throw exceptions::read_timeout();
            error = read_result.value().first;
            received = read_result.value().second;
            return received;
        }
    };

    inline std::size_t read_until(
        boost::asio::ip::tcp::socket &sock, ssl_data *ssl,
        boost::asio::streambuf &b, const char *term,
        boost::system::error_code &e
    ) {
        timeout_wrapper timeout(sock, e);
        if ( ssl )
            boost::asio::async_read_until(ssl->ssl_sock, b, term,
                timeout.read_async_function());
        else
            boost::asio::async_read_until(sock, b, term,
                timeout.read_async_function());
        return timeout.complete();
    }

    template< typename F >
    inline std::size_t read(
        boost::asio::ip::tcp::socket &sock, ssl_data *ssl,
        boost::asio::streambuf &b, F f,
        boost::system::error_code &e
    ) {
        timeout_wrapper timeout(sock, e);
        if ( ssl )
            boost::asio::async_read(ssl->ssl_sock, b, f,
                timeout.read_async_function());
        else
            boost::asio::async_read(sock, b, f,
                timeout.read_async_function());
        return timeout.complete();
    }

    inline std::size_t read_until(
        boost::asio::ip::tcp::socket &sock, ssl_data *ssl,
        boost::asio::streambuf &b, const char *term
    ) {
        boost::system::error_code error;
        std::size_t bytes = read_until(sock, ssl, b, term, error);
        handle_error(
            "read_until(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, "
                "boost::asio::streambuf &b, const char *term)",
            "Whilst reading data from a socket", error);
        return bytes;
    }
    template< typename F >
    inline std::size_t read(
        boost::asio::ip::tcp::socket &sock, ssl_data *ssl,
        boost::asio::streambuf &b, F f
    ) {
        boost::system::error_code error;
        std::size_t bytes = read(sock, ssl, b, f, error);
        handle_error(
            "read<F>(boost::asio::ip::tcp::socket &sock, ssl_data *ssl, "
                "boost::asio::streambuf &b, F f)",
            "Whilst reading data from a socket", error);
        return bytes;
    }

    void connect(
        boost::asio::io_service &io_service,
        boost::asio::ip::tcp::socket &socket,
        const host &host, port_number port
    ) {
        using namespace boost::asio::ip;
        tcp::resolver resolver(io_service);
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
            timeout_wrapper timeout(socket, connect_error, c_connect_timeout);
            socket.async_connect(*endpoint++, timeout.connect_async_function());
            try {
                timeout.complete();
            } catch ( exceptions::read_timeout &e ) {
                connect_error = boost::asio::error::timed_out;
            }
        }
        if ( connect_error )
            throw fostlib::exceptions::connect_failure(connect_error, host, port);
    }
}


fostlib::network_connection::network_connection(std::auto_ptr< boost::asio::ip::tcp::socket > socket)
: m_socket(socket), m_ssl_data(NULL) {
}

fostlib::network_connection::network_connection(const host &h, nullable< port_number > p)
: m_socket(new boost::asio::ip::tcp::socket(io_service)), m_ssl_data(NULL) {
    const port_number port = p.value(coerce< port_number >(h.service().value("0")));
    json socks(c_socks_version.value());

    if ( !socks.isnull() ) {
        const host socks_host( coerce< host >( c_socks_host.value() ) );
        connect(io_service, *m_socket, socks_host, coerce< port_number >(socks_host.service().value("0")));
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
        connect(io_service, *m_socket, h, port);
}

fostlib::network_connection::~network_connection() {
    delete m_ssl_data;
}


void fostlib::network_connection::start_ssl() {
    m_ssl_data = new ssl(io_service, *m_socket);
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
            "Could not find a \\r\\n sequence before network connection ended");
    return *this;
}
network_connection &fostlib::network_connection::operator >> (
        std::vector< utf8 > &v) {
    const std::size_t chunk = coerce<std::size_t>(c_large_read_chunk_size.value());
    while( v.size() - m_input_buffer.size()
            && read(*m_socket, m_ssl_data, m_input_buffer,
                boost::asio::transfer_at_least(
                    std::min(v.size() - m_input_buffer.size(), chunk))) );
    if ( m_input_buffer.size() < v.size() ) {
        fostlib::exceptions::unexpected_eof exception(
            "Could not read all of the requested bytes from the network connection");
        insert(exception.data(), "bytes read", coerce<int64_t>(m_input_buffer.size()));
        insert(exception.data(), "bytes expected", coerce<int64_t>(v.size()));
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
        fostlib::exceptions::read_error(error);
}


/*
    fostlib::exceptions::socket_error
*/


fostlib::exceptions::socket_error::socket_error() throw () {
}

fostlib::exceptions::socket_error::socket_error(
    boost::system::error_code error
) throw ()
: error(error) {
    info() << error << std::endl;
}

fostlib::exceptions::socket_error::~socket_error() throw ()
try {
} catch ( ... ) {
    fostlib::absorbException();
}


/*
    fostlib::exceptions::connect_failure
*/


fostlib::exceptions::connect_failure::connect_failure(
    boost::system::error_code error, const host &h, port_number p
) throw ()
: socket_error(error) {
    insert(data(), "host", h);
    insert(data(), "port", p);
}


fostlib::wliteral const fostlib::exceptions::connect_failure::message()
        const throw () {
    return L"Network connection failure";
}


/*
    fostlib::exceptions::read_timeout
*/


fostlib::exceptions::read_timeout::read_timeout() throw () {
}


fostlib::wliteral const fostlib::exceptions::read_timeout::message()
        const throw () {
    return L"Read time out";
}


/*
    fostlib::exceptions::read_error
*/


fostlib::exceptions::read_error::read_error() throw () {
}


fostlib::wliteral const fostlib::exceptions::read_error::message()
        const throw () {
    return L"Read error";
}
