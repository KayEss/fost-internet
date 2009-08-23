/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_HTTP_USERAGENT_HPP
#define FOST_INTERNET_HTTP_USERAGENT_HPP
#pragma once


#include <fost/detail/http.hpp>


namespace fostlib {


    namespace http {


        class FOST_INET_DECLSPEC user_agent : boost::noncopyable {
            public:
                user_agent();
                explicit user_agent(const url &base);


                class FOST_INET_DECLSPEC response : boost::noncopyable {
                    friend class user_agent;
                    mime::mime_headers m_headers;
                    response(
                        std::auto_ptr< network_connection > connection,
                        const string &m, const url &u,
                        const string &protocol, int status, const string &message
                    );
                    public:
                        // What we asked for
                        accessors< const string > method;
                        accessors< const url > address;

                        // What we got
                        accessors< const string > protocol;
                        accessors< const int > status;
                        accessors< const string > message;

                        const mime &body() const;
                    private:
                        std::auto_ptr< network_connection > m_cnx;
                        mutable boost::scoped_ptr< mime > m_body;
                };
                class FOST_INET_DECLSPEC request {
                    boost::shared_ptr< mime > m_data;
                    public:
                        request(const string &method, const url &url);
                        request(const string &method, const url &url, const string &data);
                        request(const string &method, const url &url, const boost::filesystem::wpath &data);

                        mime::mime_headers &headers() { return m_data->headers(); }
                        const mime::mime_headers & headers() const { return m_data->headers(); }
                        std::ostream &print_on( std::ostream &o ) const { return m_data->print_on( o ); }

                        accessors< string > method;
                        accessors< url > address;
                        mime &data() const { return *m_data; }
                };


                accessors< nullable< boost::function< void ( request & ) > > > authentication;
                accessors< url > base;


                std::auto_ptr< response > operator () (request &) const;

                std::auto_ptr< response > get( const url &url ) const {
                    request r(L"GET", url);
                    return (*this)(r);
                }
                std::auto_ptr< response > post( const url &url, const string &data ) const {
                    request r(L"POST", url, data);
                    return (*this)(r);
                }
        };


    }


}


#endif // FOST_INTERNET_HTTP_USERAGENT_HPP
