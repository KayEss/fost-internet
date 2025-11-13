#ifndef FOST_CONNECTION_HPP
#define FOST_CONNECTION_HPP
#pragma once


#include <fost/core>
#include <fost/pointers>
#include <fost/host.hpp>

#include <fost/exception/unexpected_eof.hpp>


namespace fostlib {


#if BOOST_VERSION >= 106600 // 1.66.0
    using io_context_type = boost::asio::io_context;
#else
    using io_context_type = boost::asio::io_service;
#endif
    using socket_type = boost::asio::ip::tcp::socket;


    /// ## Networking configuration options
    extern setting<int64_t> const c_connect_timeout;
    extern setting<int64_t> const c_read_timeout;
    extern setting<int64_t> const c_large_read_chunk_size;

    extern setting<json> const c_socks_version;
    extern setting<fostlib::string> const c_socks_host;

    /// TLS server certificate verification options. See the
    /// [TLS documentation](./../fost-inet/TLS.md)
    extern setting<bool> const c_always_skip_cert_verification;
    extern setting<bool> const c_tls_use_standard_verify_paths;
    extern setting<json> const c_extra_ca_cert_paths;
    extern setting<nullable<string>> const c_certificate_verification_file;
    extern setting<json> const c_extra_leaf_certificates;

    /// ### Well known certificates
    felspar::u8view digicert_root_ca();
    felspar::u8view lets_encrypt_root();


    /// ## `network_connection`

    /// A TCP/IP network connection from either a server or client
    class FOST_INET_DECLSPEC network_connection final {
        struct ssl;
        std::unique_ptr<io_context_type> io_service;
        std::unique_ptr<socket_type> m_socket;
        std::unique_ptr<boost::asio::streambuf> m_input_buffer;
        ssl *m_ssl_data;

      public:
        /// This type is not copyable
        network_connection(const network_connection &) = delete;
        network_connection &operator=(const network_connection &) = delete;

        /// Used for server end points where accept returns a socket
        network_connection(
                std::unique_ptr<io_context_type> io_service,
                std::unique_ptr<socket_type> socket);
        /// Used for clients where a host is connected to on a given port number
        network_connection(const host &h, nullable<port_number> p = null);
        /// Move constructor
        network_connection(network_connection &&);

        /// Non-virtual destructor so sub-classing is not allowed
        ~network_connection();

        /// Start SSL on this connection. After a successful handshake all
        /// traffic will be over SSL.
        void start_ssl();
        /// Start a SSL connection and verify the server connection for the
        /// specified host name.
        void start_ssl(felspar::u8view hostname, bool verify = true);

        /// Return the remote end
        host remote_end();

        /// Immediately push data over the network
        network_connection &operator<<(const const_memory_block &);
        /// Immediately push data over the network
        network_connection &operator<<(const utf8_string &s);
        /// Immediately push data over the network
        network_connection &operator<<(const std::stringstream &ss);

        /// Read up until the next \r\n which is discarded
        network_connection &operator>>(std::string &s);
        /// Read up until the next \r\n which is discarded and decode the line
        /// as UTF-8
        network_connection &operator>>(utf8_string &s);
        /// Read into the vector. The vector size must match the number of bytes
        /// expected.
        network_connection &operator>>(std::vector<utf8> &v);
        /// Read everything until the connection is dropped by the server
        void operator>>(boost::asio::streambuf &b);
    };


    namespace exceptions {


        /// The base exception type for all kinds of transmission errors
        class FOST_INET_DECLSPEC socket_error : public exception {
          public:
            /// Construct a socket error
            socket_error(
                    std::source_location const & =
                            std::source_location::current()) noexcept;
            /// Throw an exception providing a message
            socket_error(
                    const string &message,
                    std::source_location const & =
                            std::source_location::current()) noexcept;
            /// Construct a connect failure exception
            socket_error(
                    boost::system::error_code,
                    std::source_location const & =
                            std::source_location::current()) noexcept;
            /// Throw providing a message and extra information
            socket_error(
                    const string &message,
                    const string &extra,
                    std::source_location const & =
                            std::source_location::current()) noexcept;
            /// Allow us to throw from a Boost error code with a message
            socket_error(
                    boost::system::error_code error,
                    const string &message,
                    std::source_location const & =
                            std::source_location::current()) noexcept;

            /// Destruct the exception without throwing
            ~socket_error() noexcept;

            /// Allow access to the error code that caused the exception
            accessors<const nullable<boost::system::error_code>> error;

          protected:
            /// The error message title
            felspar::u8view message() const noexcept;
        };


        /// Thrown for errors during connection to a socket
        class FOST_INET_DECLSPEC connect_failure : public socket_error {
          public:
            /// Construct a connect failure exception
            connect_failure(
                    boost::system::error_code,
                    const host &,
                    port_number,
                    std::source_location const & =
                            std::source_location::current()) noexcept;

          protected:
            /// The error message title
            felspar::u8view message() const noexcept;
        };

        /// Thrown for errors during connection to a socket or reading from a
        /// socket
        class FOST_INET_DECLSPEC read_timeout : public socket_error {
          public:
            /// Construct a connect failure exception
            read_timeout(
                    std::source_location const & =
                            std::source_location::current()) noexcept;

          protected:
            /// The error message title
            felspar::u8view message() const noexcept;
        };

        /// Thrown for general errors when reading from a socket
        class FOST_INET_DECLSPEC read_error : public socket_error {
          public:
            /// Construct a connect failure exception
            read_error(
                    std::source_location const & =
                            std::source_location::current()) noexcept;
            /// Construct a read error from an error code
            read_error(
                    boost::system::error_code,
                    std::source_location const & =
                            std::source_location::current()) noexcept;

          protected:
            /// The error message title
            felspar::u8view message() const noexcept;
        };


    }


}


#endif // FOST_CONNECTION_HPP
