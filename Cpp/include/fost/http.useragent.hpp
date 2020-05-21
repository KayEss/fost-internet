/**
    Copyright 2008-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
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

            class request;
            class response;

            /// A function that will authenticate the request
            accessors<std::function<void(request &)>> authentication;
            /// The base URL used for new requests
            accessors<url> base;

            /// Perform the request and return the response
            std::unique_ptr<response> operator()(request &) const;

            /// Perform a GET request
            std::unique_ptr<response> get(const url &url) const;
            /// Perform a POST request
            template<typename D>
            std::unique_ptr<response> post(const url &url, const D &data) const;
            /// Perform a PUT request
            template<typename D>
            std::unique_ptr<response> put(const url &url, const D &data) const;
        };


        /// Describe a HTTP request
        class FOST_INET_DECLSPEC user_agent::request {
            std::shared_ptr<mime> m_data;

          public:
            /// Construct a request for a URL
            request(const string &method, const url &url);
            /// Construct a request for a URL with body data
            request(const string &method,
                    const url &url,
                    const string &data,
                    mime::mime_headers = mime::mime_headers{});
            /// Construct a request for a URL with body data from a file
            request(const string &method,
                    const url &url,
                    const fostlib::fs::path &data);
            /// Construct a request for a URL with MIME data
            request(const string &method,
                    const url &url,
                    std::shared_ptr<mime> mime_data);
            request(const string &method,
                    const url &url,
                    boost::shared_ptr<mime> mime_data)
            : request{method, url, to_std(mime_data)} {}

            /// Allow manipulation of the request headers
            mime::mime_headers &headers() { return m_data->headers(); }
            /// Allow reading of the request headers
            const mime::mime_headers &headers() const {
                return m_data->headers();
            }
            /// Print the request on a narrow stream
            std::ostream &print_on(std::ostream &o) const {
                return m_data->print_on(o);
            }

            /// The request method
            accessors<string> method;
            /// The full request URL
            accessors<url> address;
            /// The request data
            mime &data() const { return *m_data; }
        };


        /// Describe a HTTP response
        class FOST_INET_DECLSPEC user_agent::response : boost::noncopyable {
            friend class user_agent;
            mime::mime_headers m_headers;
            response(
                    network_connection &&connection,
                    const string &m,
                    const url &u,
                    const string &protocol,
                    int status,
                    const string &message);

          public:
            /// Build a response, normally for testing purposes
            response(
                    const string &method,
                    const url &address,
                    int status,
                    boost::shared_ptr<binary_body> body,
                    const mime::mime_headers & = mime::mime_headers(),
                    const string &message = string());

            /// The request method
            accessors<const string> method;
            /// The request URL
            accessors<const url> address;

            /// The response protocol
            accessors<const string> protocol;
            /// The response status
            accessors<const int> status;
            /// The response message text
            accessors<const string> message;

            /// Allow reading of the request headers
            const mime::mime_headers &headers() const { return m_headers; }

            /// The response body and headers
            boost::shared_ptr<binary_body> body();

          private:
            std::unique_ptr<network_connection> m_cnx;
            boost::shared_ptr<binary_body> m_body;
        };


        /// Perform a GET request
        inline std::unique_ptr<user_agent::response>
                user_agent::get(const url &url) const {
            request r("GET", url);
            return (*this)(r);
        }


        /// Perform a POST request
        template<typename D>
        inline std::unique_ptr<user_agent::response>
                user_agent::post(const url &url, const D &data) const {
            request r("POST", url, data);
            return (*this)(r);
        }


        /// Perform a PUT request
        template<typename D>
        inline std::unique_ptr<user_agent::response>
                user_agent::put(const url &url, const D &data) const {
            request r("PUT", url, data);
            return (*this)(r);
        }


    }


}


#endif // FOST_INTERNET_HTTP_USERAGENT_HPP
