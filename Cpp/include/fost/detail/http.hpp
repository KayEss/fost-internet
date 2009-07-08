/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_HTTP_HPP
#define FOST_INTERNET_HTTP_HPP
#pragma once


#include <fost/detail/url.hpp>
#include <fost/detail/mime.hpp>


namespace fostlib {


    namespace http {


        class FOST_INET_DECLSPEC request : boost::noncopyable {
        public:
            request( std::auto_ptr< asio::tcpsocket > socket );

            const string &method();
            const string &file_spec();

            void operator() ( const mime &response );

        private:
            std::auto_ptr< asio::tcpsocket > m_sock;
            nullable< std::pair< string, string > > m_first_line;
        };


    }


}


#endif // FOST_INTERNET_HTTP_HPP
