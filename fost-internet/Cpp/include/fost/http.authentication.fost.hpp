/*
    Copyright 2009-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_HTTP_AUTHENTICATION_FOST_HPP
#define FOST_HTTP_AUTHENTICATION_FOST_HPP
#pragma once


#include "http.useragent.hpp"


namespace fostlib {


    namespace http {


        /// Signs a request with the specified key and secret.
        void FOST_INET_DECLSPEC fost_authentication(
            const string &api_key, const string &secret,
            const std::set< string > &headers_to_sign,
            user_agent::request &request
        );
        /// Adds authentication to the specified user agent
        void FOST_INET_DECLSPEC fost_authentication(
            user_agent &ua,
            const string &api_key, const string &secret,
            const std::set< string > &headers_to_sign = std::set< string >()
        );


    }


}


#endif // FOST_HTTP_AUTHENTICATION_FOST_HPP
