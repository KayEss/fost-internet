/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_DETAIL_S3_HPP
#define FOST_DETAIL_S3_HPP
#pragma once


#include <fost/crypto>
#include <fost/internet>
#include "http.useragent.hpp"


namespace fostlib {


    namespace aws {


        class FOST_AWS_DECLSPEC s3 {
        public:
            s3();
            explicit s3( const string &name );

            void put( const boost::filesystem::wpath &file, const url &location ) const;
        };


    }


}


#endif // FOST_DETAIL_S3_HPP
