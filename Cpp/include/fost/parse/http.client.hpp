/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/parse/url.hpp>


namespace fostlib {


    namespace http {


        struct response_status {
            std::string version;
            int status;
            std::string message;
        };


        template<typename Iterator>
        struct client_first_line : boost::spirit::qi::grammar<Iterator, response_status()> {
            boost::spirit::qi::rule<Iterator, response_status()> top;
            boost::spirit::qi::rule<Iterator, std::string()> version, message;
            boost::spirit::qi::rule<Iterator, int> status;
            boost::spirit::qi::rule<Iterator, void()> space;

            client_first_line()
            : client_first_line::base_type(top) {
                using boost::spirit::qi::_1;
                using boost::spirit::qi::_2;
                using boost::spirit::qi::_3;
                using boost::spirit::qi::_val;

                space = +boost::spirit::qi::lit(' ');

                top = (version >> space >> status >> space >> message)
                    [boost::phoenix::bind([](auto &l, auto &v, auto &s, auto &m) {
                            l = {v, s, m};
                        }, _val, _1, _2, _3)];

                version = boost::spirit::qi::string("HTTP/0.9")
                    | boost::spirit::qi::string("HTTP/1.0")
                    | boost::spirit::qi::string("HTTP/1.1");
                status = boost::spirit::qi::uint_parser<int, 10, 3, 3>();
                message = +boost::spirit::qi::char_;
            }
        };


    }


}

