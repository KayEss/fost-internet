/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/parse/host.hpp>


namespace fostlib {


    /// Parse SMTP addresses (that more or less conform to RFC822).
    template<typename Iterator>
    struct smtp_address_parser :
        public boost::spirit::qi::grammar<Iterator, std::pair<boost::optional<std::string>, std::string>()>
    {
        boost::spirit::qi::rule<Iterator, std::pair<boost::optional<std::string>, std::string>()> top, address;
        boost::spirit::qi::rule<Iterator, std::string()> name, rfc822, user;
        hostname_parser<Iterator> hostname;

        smtp_address_parser()
        : smtp_address_parser::base_type(top) {
            using boost::spirit::qi::_1;
            using boost::spirit::qi::_2;
            using boost::spirit::qi::_val;

            top =
                rfc822
                    [boost::phoenix::bind([](auto &v, auto &s) {
                        v = std::make_pair(boost::optional<std::string>(), s);
                    }, _val, _1)]
                | address
                    [boost::phoenix::bind([](auto &v, auto &s) {
                        v = s;
                    }, _val, _1)];

            address = (name >> boost::spirit::qi::lit('<') >> rfc822 > boost::spirit::qi::lit('>'))
                [boost::phoenix::bind([](auto &v, auto &n, auto &a) {
                    v = std::make_pair(n, a);
                }, _val, _1, _2)];
            rfc822 = user >> boost::spirit::qi::char_('@') >> hostname;
            user = +boost::spirit::qi::char_("a-zA-Z0-9.,_+-");
            name = +(boost::spirit::qi::char_ - boost::spirit::qi::char_('<'));
        }
    };


}

