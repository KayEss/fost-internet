/*
    Copyright 2008-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_HTTP_SERVER_HPP
#define FOST_INTERNET_HTTP_SERVER_HPP
#pragma once


#include <fost/url.hpp>
#include <fost/http.hpp>


namespace fostlib {


    namespace http {


        /// A minimal HTTP server
        class FOST_INET_DECLSPEC server : boost::noncopyable {
        public:
            /// The request from a user agent
            class FOST_INET_DECLSPEC request : boost::noncopyable {
                friend class fostlib::http::server;
                boost::scoped_ptr< network_connection > m_cnx;
                string m_method; url::filepath_string m_pathspec;
                boost::scoped_ptr< mime > m_mime;

                public:
                    /// This constructor initialises a server request from a socket connection
                    request( std::auto_ptr< boost::asio::ip::tcp::socket > connection );
                    /// This constructor is useful for mocking the request for code that interacts with a server
                    request(
                        const string &method, const url::filepath_string &filespec,
                        std::auto_ptr< mime > headers_and_body
                    );

                    /// The request method
                    const string &method() const { return m_method; }
                    /// The requested resource
                    const url::filepath_string &file_spec() const { return m_pathspec; }
                    /// The request body and headers
                    const mime &data() const;

                    /// Used to pass the response back to the user agent.This will throw on a mocked connection
                    void operator () ( const mime &response );
            };

            /// Create a server bound to a host and port
            explicit server( const host &h, uint16_t port = 80 );

            /// The host the server is bound to
            accessors< const host > binding;
            /// The port the server is bound to
            accessors< const uint16_t > port;

            /// Return the next request on the underlying socket
            std::auto_ptr< request > operator() ();
            /// Run the provided lambda to service requests forever
            void operator () ( boost::function< bool ( request & ) > service_lambda );

        private:
            boost::asio::io_service m_service;
            boost::asio::ip::tcp::acceptor m_server;
        };


    }


}


#endif // FOST_INTERNET_HTTP_SERVER_HPP
