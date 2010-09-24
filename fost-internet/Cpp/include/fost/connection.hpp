/*
    Copyright 2008-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_CONNECTION_HPP
#define FOST_CONNECTION_HPP
#pragma once


#include <fost/core>
#include <fost/pointers>
#include <fost/host.hpp>


namespace fostlib {


    /// A TCP/IP network connection from either a server or client
    class FOST_INET_DECLSPEC network_connection : boost::noncopyable {
        struct ssl;
        boost::asio::io_service io_service;
        std::auto_ptr< boost::asio::ip::tcp::socket > m_socket;
        boost::asio::streambuf m_input_buffer;
        ssl *m_ssl_data;
    public:
        /// Used for server end points where accept returns a socket
        network_connection(std::auto_ptr< boost::asio::ip::tcp::socket > socket);
        /// Used for clients where a host is connected to on a given port number
        network_connection(const host &h, nullable< port_number > p = null);

        /// Non-virtual destructor os sub-classing is not allowed
        ~network_connection();


        /// Start SSL on this connection. After a succesful handshake all traffic will be over SSL.
        void start_ssl();

        /// Immediately push data over the network
        network_connection &operator << ( const const_memory_block & );
        /// Immediately push data over the network
        network_connection &operator << ( const utf8_string &s );
        /// Immediately push data over the network
        network_connection &operator << ( const std::stringstream &ss );

        /// Read up until the next \r\n which is discarded
        network_connection &operator >> ( std::string &s );
        /// Read up until the next \r\n which is discarded and decode the line as UTF-8
        network_connection &operator >> ( utf8_string &s );
        /// Read into the vector. The vector size must match the number of bytes expected.
        network_connection &operator >> ( std::vector< utf8 > &v );
        /// Read everything until the connection is dropped by the server
        void operator >> ( boost::asio::streambuf &b );
    };


    namespace exceptions {


        /// Thrown for errors during connection to a socket
        class FOST_INET_DECLSPEC connect_failure : public exception {
        public:
            /// Construct a connect failure exception
            connect_failure(boost::system::error_code) throw();

            /// Allow access to the error code that caused the exception
            accessors< const boost::system::error_code > error;

        protected:
            /// The error message title
            const wchar_t * const message() const throw ();
        };


    }


}


#endif // FOST_CONNECTION_HPP
