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


        struct request_status {
            std::string method;
            url::filepath_string path;
            enum version_n {
                http_9 = 9,
                http_10 = 10,
                http_11 = 11
            } version = http_9;
            url::query_string query;
        };


        template<typename Iterator>
        struct server_first_line :
        boost::spirit::qi::grammar<Iterator, request_status()> {
            boost::spirit::qi::rule<Iterator, request_status()> top;
            boost::spirit::qi::rule<Iterator, std::string()> method;
            url_filespec_parser<Iterator> filespec;
            query_string_parser<Iterator> query;
            boost::spirit::qi::symbols<char, request_status::version_n> version;
            boost::spirit::qi::rule<Iterator, void()> spaces;

            server_first_line() : server_first_line::base_type(top) {
                using boost::spirit::qi::_1;
                using boost::spirit::qi::_2;
                using boost::spirit::qi::_3;
                using boost::spirit::qi::_4;
                using boost::spirit::qi::_val;

                spaces = *boost::spirit::qi::lit(' ');

                top = (method >> spaces >> filespec
                       >> -(boost::spirit::qi::lit('?') >> query)
                       >> -(spaces >> version))[boost::phoenix::bind(
                        [](auto &l, auto m, auto f, auto qs, auto v) {
                            l = request_status{
                                    m, f, v.value_or(request_status::http_9)};
                            if (qs) l.query = qs.value();
                        },
                        _val, _1, _2, _3, _4)];

                method = +boost::spirit::qi::char_('A', 'Z');
                version.add("HTTP/1.0", request_status::http_10)(
                        "HTTP/1.1", request_status::http_11);
            }
        };


    }


}
