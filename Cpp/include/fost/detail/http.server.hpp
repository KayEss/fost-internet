/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_HTTP_SERVER_HPP
#define FOST_INTERNET_HTTP_SERVER_HPP
#pragma once


#include <fost/detail/http.hpp>


namespace fostlib {


    namespace http {


        class FOST_INET_DECLSPEC server : boost::noncopyable {
        public:
            class FOST_INET_DECLSPEC request : boost::noncopyable {
                friend class fostlib::http::server;
                request( std::auto_ptr< boost::asio::ip::tcp::socket > connection );
            public:

                const string &method();
                const string &file_spec();

                void operator() ( const mime &response );

            private:
                std::auto_ptr< network_connection > m_cnx;
                nullable< std::pair< string, string > > m_first_line;
            };

            explicit server( const host &h, uint16_t port = 80 );

            accessors< const host > binding;
            accessors< const uint16_t > port;

            std::auto_ptr< request > operator() ();

        private:
            boost::asio::io_service m_service;
            boost::asio::ip::tcp::acceptor m_server;
        };


    }


}


#endif // FOST_INTERNET_HTTP_SERVER_HPP
