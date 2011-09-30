/*
    Copyright 2008-2011, Felspar Co Ltd. http://support.felspar.com/
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
                boost::function<void (const mime&, const ascii_string&)> m_handler;
                string m_method;
                url::filepath_string m_pathspec;
                nullable< ascii_printable_string > m_query_string;
                boost::shared_ptr< binary_body > m_mime;

                public:
                    /// Create an empty request
                    request();
                    /// Create a request from data on the provided socket
                    request(std::auto_ptr< boost::asio::ip::tcp::socket > connection);
                    /// This constructor is useful for mocking the request that doesn't get responded to
                    request(
                        const string &method, const url::filepath_string &filespec,
                        std::auto_ptr< binary_body > headers_and_body);
                    /// This constructor is useful for mocking the request that gets responded to
                    request(
                        const string &method, const url::filepath_string &filespec,
                        std::auto_ptr< binary_body > headers_and_body,
                        boost::function<void (const mime&, const ascii_string &)>);

                    /// Parse a request on the provided socket
                    void operator () (
                        std::auto_ptr< boost::asio::ip::tcp::socket > connection);

                    /// The request method
                    const string &method() const { return m_method; }
                    /// The requested resource
                    const url::filepath_string &file_spec() const { return m_pathspec; }
                    /// The query string
                    const nullable< ascii_printable_string > query_string() const {
                        return m_query_string;
                    }
                    /// The request body and headers
                    boost::shared_ptr< binary_body > data() const;

                    /// Used to pass the response back to the user agent.
                    void operator () (
                        const mime &response,
                        const int status = 200);
                    /// Used to pass the response back to the user agent.
                    void operator () (
                        const mime &response,
                        const ascii_string &status_text);
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
