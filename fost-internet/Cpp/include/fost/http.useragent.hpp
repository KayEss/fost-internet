/*
    Copyright 2008-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_HTTP_USERAGENT_HPP
#define FOST_INTERNET_HTTP_USERAGENT_HPP
#pragma once


#include <fost/http.hpp>


namespace fostlib {


    namespace http {


        class FOST_INET_DECLSPEC user_agent : boost::noncopyable {
            public:
                /// Construct a new user agent
                user_agent();
                /// Construct a new user agent given a base URL
                explicit user_agent(const url &base);


                /// Describe a HTTP response
                class FOST_INET_DECLSPEC response : boost::noncopyable {
                    friend class user_agent;
                    mime::mime_headers m_headers;
                    response(
                        std::auto_ptr< network_connection > connection,
                        const string &m, const url &u,
                        const string &protocol, int status, const string &message
                    );
                    public:
                        /// The request method
                        accessors< const string > method;
                        /// The request URL
                        accessors< const url > address;

                        /// The response protocol
                        accessors< const string > protocol;
                        /// The response status
                        accessors< const int > status;
                        /// The response message text
                        accessors< const string > message;

                        /// The response body
                        const binary_body &body();

                    private:
                        std::auto_ptr< network_connection > m_cnx;
                        boost::scoped_ptr< binary_body > m_body;
                };
                /// Describe a HTTP request
                class FOST_INET_DECLSPEC request {
                    boost::shared_ptr< mime > m_data;
                    public:
                        /// Construct a request for a URL
                        request(const string &method, const url &url);
                        /// Construct a request for a URL with body data
                        request(const string &method, const url &url, const string &data);
                        /// Construct a request for a URL with body data from a file
                        request(
                            const string &method, const url &url,
                            const boost::filesystem::wpath &data
                        );

                        /// Allow manipulation of the request headers
                        mime::mime_headers &headers() {
                            return m_data->headers();
                        }
                        /// Allow reading of the request headers
                        const mime::mime_headers & headers() const {
                            return m_data->headers();
                        }
                        /// Print the request on a narrow stream
                        std::ostream &print_on( std::ostream &o ) const {
                            return m_data->print_on( o );
                        }

                        /// The request method
                        accessors< string > method;
                        /// The full request URL
                        accessors< url > address;
                        /// The request data
                        mime &data() const { return *m_data; }
                };


                /// A function that will authenticate the request
                accessors< nullable<
                    boost::function< void ( request & ) >
                > > authentication;
                /// The base URL used for new requests
                accessors< url > base;


                /// Perform the request and return the response
                std::auto_ptr< response > operator () (request &) const;

                /// Perform a GET request
                std::auto_ptr< response > get( const url &url ) const {
                    request r(L"GET", url);
                    return (*this)(r);
                }
                /// Perform a POST request
                std::auto_ptr< response > post( const url &url, const string &data ) const {
                    request r(L"POST", url, data);
                    return (*this)(r);
                }
                /// Perform a PUT request
                std::auto_ptr< response > put( const url &url, const string &data ) const {
                    request r(L"PUT", url, data);
                    return (*this)(r);
                }
        };


    }


}


#endif // FOST_INTERNET_HTTP_USERAGENT_HPP
