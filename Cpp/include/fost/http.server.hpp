/*
    Copyright 2008-2017, Felspar Co Ltd. http://support.felspar.com/
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
                std::function<void(mime&, const ascii_string&)> m_handler;
                string m_method;
                url::filepath_string m_pathspec;
                url::query_string m_query_string;
                boost::shared_ptr< binary_body > m_mime;

                public:
                    /// Create an empty request
                    request();
                    /// Create a request from data on the provided socket
                    request(
                        std::unique_ptr< boost::asio::io_service > io_service,
                        std::unique_ptr<boost::asio::ip::tcp::socket> connection);
                    /// This constructor is useful for mocking the request that doesn't get responded to
                    request(
                        const string &method, const url::filepath_string &filespec,
                        std::unique_ptr< binary_body > headers_and_body
                            = std::unique_ptr< binary_body >(),
                        const url::query_string &qs = url::query_string());
                    /// This constructor is useful for mocking the request that doesn't get responded to
                    request(
                        const string &method, const url::filepath_string &filespec,
                        const url::query_string &qs,
                        std::unique_ptr< binary_body > headers_and_body
                            = std::unique_ptr< binary_body >());
                    /// This constructor is useful for mocking the request that gets responded to
                    request(
                        const string &method, const url::filepath_string &filespec,
                        std::unique_ptr< binary_body > headers_and_body,
                        std::function<void(const mime&, const ascii_string &)>);

                    /// The request method
                    const string &method() const { return m_method; }
                    /// The requested resource
                    const url::filepath_string &file_spec() const { return m_pathspec; }
                    /// The query string
                    const url::query_string &query_string() const {
                        return m_query_string;
                    }
                    /// The request body and headers
                    boost::shared_ptr< binary_body > data() const;

                    /// Shortcut to the headers
                    mime::mime_headers &headers() {
                        return m_mime->headers();
                    }
                    /// Shortcut to the headers
                    const mime::mime_headers &headers() const {
                        return m_mime->headers();
                    }

                    /// Return the remote host address. The setting
                    /// webserver:remote_header is used to retrieve this.
                    /// The remote end will always be set in the
                    /// __remote_addr header when the request is parsed
                    /// from the socket, but the setting should be changed
                    /// when the real IP address is passed in through some
                    /// other header like `X-Forwarded-For`
                    host remote_address() const;

                    /// Allow access to parts of the request using a jcursor.
                    /// Return null if the location doesn't exist, but throw
                    /// if the location couldn't possibly exist. The value
                    /// may exist and return an empty JSON value
                    nullable<json> operator [] (const jcursor &) const;

                    /// Used to pass the response back to the user agent.
                    void operator () (
                        mime &response,
                        const int status = 200);
                    /// Used to pass the response back to the user agent.
                    void operator () (
                        mime &response,
                        const ascii_string &status_text);
            };

            /// Create a server bound to a host and port
            explicit server( const host &h, uint16_t port = 80 );

            /// The host the server is bound to
            accessors< const host > binding;
            /// The port the server is bound to
            accessors< const uint16_t > port;

            /// Return the next request on the underlying socket
            std::unique_ptr<request> operator() ();
            /// Run the provided lambda to service requests forever
            void operator () (
                std::function<bool(request &)> service_lambda);
            /// Run the provided lambda to service requests until the terminator return true
            void operator () (
                std::function<bool(request &)> service_lambda,
                std::function<bool(void)> terminator);

            /// Return the status text associated with a status code
            static nliteral status_text( int code );

        private:
            boost::asio::io_service m_service;
            boost::asio::ip::tcp::acceptor m_server;
        };


    }


}


#endif // FOST_INTERNET_HTTP_SERVER_HPP
