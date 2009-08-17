/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
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


        void fost_authentication(
            const fostlib::ascii_string &api_key,
            const fostlib::ascii_string &secret,
            user_agent::request &request
        );


    }


}


#endif // FOST_HTTP_AUTHENTICATION_FOST_HPP
