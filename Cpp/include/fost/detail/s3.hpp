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
#include <fost/http>


namespace fostlib {


    namespace aws {


        namespace s3 {


            void FOST_AWS_DECLSPEC rest_authentication(
                const string &account, const ascii_string &bucket, http::user_agent::request &request
            );


            class FOST_AWS_DECLSPEC file_info {
                boost::shared_ptr< http::user_agent::response > m_response;
                public:
                    file_info(const http::user_agent &, const ascii_string &bucket, const boost::filesystem::wpath &);

                    accessors< const boost::filesystem::wpath > path;

                    bool exists() const;
                    nullable< string > md5() const;
            };

            class FOST_AWS_DECLSPEC bucket {
                http::user_agent m_ua;
                public:
                    static const setting< string > s_account_name;

                    explicit bucket(const ascii_string &name);

                    accessors< const ascii_string > name;

                    file_info stat(const boost::filesystem::wpath &) const;
                    void put(const boost::filesystem::wpath &file, const boost::filesystem::wpath &location) const;
            };


        }


    }


}


#endif // FOST_DETAIL_S3_HPP
