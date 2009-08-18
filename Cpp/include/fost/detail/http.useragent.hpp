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


                class FOST_INET_DECLSPEC response : public mime {
                    friend class user_agent;
                    response(
                        std::auto_ptr< network_connection > connection,
                        const string &m, const url &u
                    );
                    public:
                        accessors< const string > method;
                        accessors< const url > location;

                        std::auto_ptr< mime > body();
                    private:
                        std::auto_ptr< network_connection > m_cnx;
                };
                class FOST_INET_DECLSPEC request : public text_body {
                    public:
                        request(const string &method, const url &url, const nullable< string > &data  = null);

                        accessors< const string > method;
                        accessors< const url > address;
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
