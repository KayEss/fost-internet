/*
    Copyright 1999-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_SMTP_HPP
#define FOST_INTERNET_SMTP_HPP
#pragma once


#include <fost/string>
#include <set>
#include <fost/detail/host.hpp>
#include <fost/detail/mime.hpp>


namespace fostlib {


    struct FOST_INET_DECLSPEC rfc822_address_tag {
        static void do_encode( fostlib::nliteral from, ascii_string &into );
        static void do_encode( const ascii_string &from, ascii_string &into );
        static void check_encoded( const ascii_string &s );
    };
    typedef tagged_string< rfc822_address_tag, ascii_string > rfc822_address;

    struct FOST_INET_DECLSPEC email_address {
        fostlib::accessors< rfc822_address > email;
        fostlib::accessors< nullable< string > > name;

        email_address();
        email_address( const rfc822_address &address, const nullable< string > &name = null );
        email_address(
            const ascii_string &address,
            const nullable< string > &name = null
        );
    };


    template<>
    struct FOST_INET_DECLSPEC coercer< string, email_address > {
        string coerce( const email_address & );
    };
    template<>
    struct FOST_INET_DECLSPEC coercer< email_address, string > {
        email_address coerce( const string & );
    };

    template<>
    struct FOST_INET_DECLSPEC coercer< utf8_string, email_address > {
        utf8_string coerce( const email_address &e ) {
            return fostlib::coerce< utf8_string >( fostlib::coerce< string >( e ) );
        }
    };


    class FOST_INET_DECLSPEC smtp_client : boost::noncopyable {
        struct implementation;
        implementation *m_impl;
        public:
            smtp_client( const host &server );
            ~smtp_client();

            void send( const mime &email, const rfc822_address &to, const rfc822_address &from );
    };


}


namespace std {
    template<>
    struct less< fostlib::email_address > : public std::binary_function< bool, fostlib::email_address, fostlib::email_address > {
        bool operator ()( const fostlib::email_address &l, const fostlib::email_address &r ) const {
            return less< fostlib::rfc822_address >()( l.email(), r.email() );
        }
    };
}


#endif // FOST_INTERNET_SMTP_HPP
