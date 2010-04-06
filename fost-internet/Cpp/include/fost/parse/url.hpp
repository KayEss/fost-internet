/*
    Copyright 2007-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_PARSE_URL_HPP
#define FOST_PARSE_URL_HPP


#include <fost/url.hpp>
#include <fost/parse/host.hpp>


namespace fostlib {


    namespace detail {


        struct query_string_closure : boost::spirit::closure< query_string_closure,
            url::query_string,
            utf8_string,
            utf8_string
        > {
            member1 qs;
            member2 key;
            member3 value;
        };
        struct query_string_inserter {
            template <typename Container, typename Key, typename Value>
            struct result {
                typedef void type;
            };
            template <typename Container, typename Key, typename Value>
            void operator()(Container& c, Key const& key, Value const& value) const {
                c.append( coerce< string >( key ), value.empty() ? nullable< string >() : coerce< string >( value ) );
            }
        };
        const phoenix::function<query_string_inserter> query_string_insert = query_string_inserter();

        struct url_closure : boost::spirit::closure< url_closure,
            url,
            ascii_printable_string,
            host
        > {
            member1 url;
            member2 moniker;
            member3 host;
        };

        struct url_filespec_closure : boost::spirit::closure< url_filespec_closure,
            ascii_printable_string
        > {
            member1 filespec;
        };


    }


    struct FOST_INET_DECLSPEC query_string_parser : public boost::spirit::grammar<
        query_string_parser, detail::query_string_closure::context_t
    > {
        template< typename scanner_t >
        struct definition {
            definition( query_string_parser const& self ) {
                top = !boost::spirit::list_p( (
                        key[ self.key = phoenix::arg1 ] >>
                        boost::spirit::chlit< wchar_t >( '=' )[ self.value = utf8_string() ] >>
                        !value[ self.value = phoenix::arg1 ]
                    )[
                        detail::query_string_insert( self.qs, self.key, self.value )
                    ], boost::spirit::chlit< wchar_t >( '&' )
                );
                key = ( +boost::spirit::chset<>( L"_@a-zA-Z0-9.+*-" )[
                    parsers::push_back( key.buffer, phoenix::arg1 )
                ] )[
                    key.text = parsers::coerce< utf8_string >()( key.buffer )
                ];
                value = ( +boost::spirit::chset<>( L"/:_@a-zA-Z0-9.,%+*-" )[
                    parsers::push_back( value.buffer, phoenix::arg1 )
                ] )[
                    value.text = parsers::coerce< utf8_string >()( value.buffer )
                ];
            }
            boost::spirit::rule< scanner_t > top;
            boost::spirit::rule< scanner_t, utf8_string_builder_closure::context_t > key, value;

            boost::spirit::rule< scanner_t > const &start() const { return top; }
        };
    };


    struct FOST_INET_DECLSPEC url_hostpart_parser : public boost::spirit::grammar <
        url_hostpart_parser, detail::url_closure::context_t
    > {
        template< typename scanner_t >
        struct definition {
            definition( url_hostpart_parser const &self ) {
                top = (
                        moniker[ self.moniker = phoenix::arg1 ]
                        >> boost::spirit::chlit< wchar_t >( ':' )
                        >> boost::spirit::strlit< wliteral >( L"//" )
                        >> host_p[ self.host = phoenix::arg1 ]
                    )[ self.url = phoenix::construct_< fostlib::url >( self.moniker, self.host ) ];

                moniker = ( +boost::spirit::chset<>( L"a-zA-Z+" )[
                    parsers::push_back( moniker.buffer, phoenix::arg1 )
                ] )[
                    moniker.text = parsers::coerce< ascii_printable_string >()( moniker.buffer )
                ];
            }
            host_parser host_p;

            boost::spirit::rule< scanner_t > top;
            boost::spirit::rule< scanner_t, ascii_printable_string_builder_closure::context_t > moniker;

            boost::spirit::rule< scanner_t > const &start() const { return top; }
        };
    };


    struct FOST_INET_DECLSPEC url_filespec_parser : public boost::spirit::grammar <
        url_filespec_parser, detail::url_filespec_closure::context_t
    > {
        template< typename scanner_t >
        struct definition {
            definition( url_filespec_parser const &self ) {
                top = (
                    +boost::spirit::chset<>( "_@~a-zA-Z0-9/.,:()+%*-" )
                )[ self.filespec = phoenix::construct_<ascii_printable_string>(phoenix::arg1, phoenix::arg2) ];
            }

            url_hostpart_parser url_hostpart_p;
            query_string_parser query_string_p;

            boost::spirit::rule< scanner_t > top;

            boost::spirit::rule< scanner_t > const &start() const { return top; }
        };
    };


}


#endif // FOST_PARSE_URL_HPP
