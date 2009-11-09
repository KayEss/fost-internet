/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_DETAIL_CONNECTION_HPP
#define FOST_DETAIL_CONNECTION_HPP
#pragma once


#include <fost/core>
#include <fost/pointers>
#include <fost/detail/host.hpp>


namespace fostlib {


    class FOST_INET_DECLSPEC network_connection : boost::noncopyable {
        struct ssl;
        std::auto_ptr< boost::asio::ip::tcp::socket > m_socket;
        boost::asio::streambuf m_input_buffer;
        ssl *m_ssl_data;
    public:
        // Used for server end points where accept returns a socket
        network_connection(std::auto_ptr< boost::asio::ip::tcp::socket > socket);
        // Used for clients where a host is connected to on a given port number
        network_connection(const host &h, nullable< port_number > p = null);

        // Not sure if the class should be sub-classed, but in any case we need a destructor
        virtual ~network_connection();

        /*
            Start SSL on this connection.
            After a succesful handshake all traffic will be over SSL.
        */
        void start_ssl();

        /*
            These methods just drop the data straight out to the network.
        */
        network_connection &operator << ( const const_memory_block & );
        network_connection &operator << ( const utf8_string &s );
        network_connection &operator << ( const std::stringstream &ss );

        /*
            These methods allow the socket to be read from.
        */
        // Read up until the next \r\n which is discarded
        network_connection &operator >> ( std::string &s );
        network_connection &operator >> ( utf8_string &s );
        // Read into the vector. The vector size must match the number of bytes expected.
        network_connection &operator >> ( std::vector< utf8 > &v );
        // Read everything until the connection is dropped by the server
        void operator >> ( boost::asio::streambuf &b );
    };


}


#endif // FOST_DETAIL_CONNECTION_HPP
