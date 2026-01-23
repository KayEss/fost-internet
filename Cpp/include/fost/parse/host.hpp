#ifndef FOST_PARSE_HOST_HPP
#define FOST_PARSE_HOST_HPP


#include <fost/host.hpp>
#include <fost/parse/parse.hpp>

#include <boost/phoenix.hpp>


namespace fostlib {


    template<typename Iterator>
    struct hostname_parser :
    public boost::spirit::qi::grammar<Iterator, std::string()> {
        boost::spirit::qi::rule<Iterator, std::string()> top, fqname, hname;

        hostname_parser() : hostname_parser::base_type(top) {
            top = fqname | hname;
            hname = boost::spirit::qi::char_("a-zA-Z0-9")
                    >> *boost::spirit::qi::char_("a-zA-Z0-9-");
            fqname = hname >> +(boost::spirit::qi::char_('.') >> hname);
        }
    };


    template<typename Iterator>
    struct host_parser : public boost::spirit::qi::grammar<Iterator, host()> {
        boost::spirit::qi::rule<Iterator, host()> top;
        hostname_parser<Iterator> hostname;
        boost::spirit::qi::rule<Iterator, uint32_t> rawipv4;
        boost::spirit::qi::rule<Iterator, uint16_t> port;

        host_parser() : host_parser::base_type(top) {
            using boost::spirit::qi::_1;
            using boost::spirit::qi::_2;
            using boost::spirit::qi::_val;

            top = ((hostname.fqname | rawipv4 | hostname.hname)
                   >> -port)[boost::phoenix::bind(
                    [&](auto &val, auto s, auto p) {
                        if (uint32_t *ip4 = boost::get<uint32_t>(&s)) {
                            val = p ? host(*ip4, coerce<string>(p.value()))
                                    : host(*ip4);
                        } else {
                            auto &h = boost::get<std::string>(s);
                            val = p ? host(h, p.value()) : host(h);
                        }
                    },
                    _val, _1, _2)];

            rawipv4 = boost::spirit::qi::int_;

            port = boost::spirit::qi::lit(':') >> boost::spirit::qi::int_;
        }
    };


    template<typename I>
    inline auto host_p(I begin, I end, host &into) {
        host_parser<I> rule;
        return boost::spirit::qi::parse(begin, end, rule, into) && begin == end;
    }


}


#endif // FOST_PARSE_HOST_HPP
