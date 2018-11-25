/*
    Copyright 1999-2012, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_SMTP_HPP
#define FOST_INTERNET_SMTP_HPP
#pragma once


#include <fost/string>
#include <set>
#include <fost/host.hpp>
#include <fost/mime.hpp>


namespace fostlib {


    /// Tag type for an email address
    struct FOST_INET_DECLSPEC rfc822_address_tag {
        static void
                do_encode(fostlib::nliteral from, ascii_printable_string &into);
        static void do_encode(
                const ascii_printable_string &from,
                ascii_printable_string &into);
        static void check_encoded(const ascii_printable_string &s);
    };
    /// A string type for email addresses
    typedef tagged_string<rfc822_address_tag, ascii_printable_string>
            rfc822_address;

    /// A full email address with recipient name
    struct FOST_INET_DECLSPEC email_address {
        fostlib::accessors<rfc822_address> email;
        fostlib::accessors<nullable<string>> name;

        email_address();
        email_address(
                const rfc822_address &address,
                const nullable<string> &name = null);
        email_address(
                const ascii_printable_string &address,
                const nullable<string> &name = null);
    };


    /// Returns true if the email message is a bounce delivery report (NDR)
    FOST_INET_DECLSPEC bool email_is_an_ndr(const text_body &);

    /// Returns true if the email message is a delayed delivery report
    FOST_INET_DECLSPEC bool email_is_a_delay_report(const text_body &);

    /// Returns true if the email message is an "Out of office" or vacation
    /// auto-response
    FOST_INET_DECLSPEC bool email_is_out_of_office(const text_body &);


    /// Allow a full email address to be coerced to a string
    template<>
    struct FOST_INET_DECLSPEC coercer<string, email_address> {
        string coerce(const email_address &);
    };
    /// Allow a full email address to be coerced to JSON
    template<>
    struct coercer<json, email_address> {
        json coerce(const email_address &a) {
            return fostlib::json(fostlib::coerce<fostlib::string>(a));
        }
    };
    /// Allow a string to be interpreted as an email address
    template<>
    struct FOST_INET_DECLSPEC coercer<email_address, string> {
        email_address coerce(const string &);
    };
    /// Allow a JSON blob to be interpreted as an email address
    template<>
    struct FOST_INET_DECLSPEC coercer<email_address, json> {
        email_address coerce(const json &js) {
            return fostlib::coerce<fostlib::email_address>(
                    fostlib::coerce<fostlib::string>(js));
        }
    };

    /// Allow email aadresses to be turned into UTF-8 strings
    template<>
    struct FOST_INET_DECLSPEC coercer<utf8_string, email_address> {
        utf8_string coerce(const email_address &e) {
            return fostlib::coerce<utf8_string>(fostlib::coerce<string>(e));
        }
    };


    /// An SMTP client for sending emails
    class FOST_INET_DECLSPEC smtp_client : boost::noncopyable {
        struct implementation;
        implementation *m_impl;

      public:
        /// Create a client which connects to the specified server
        smtp_client(const host &server, const port_number = 25);
        ~smtp_client();

        /// Send an email to the specified recipient from the specified sender
        void
                send(const mime &email,
                     const rfc822_address &to,
                     const rfc822_address &from);
    };


}


namespace std {
    /// Allow email addresses to be used as keys in std::map and to be stored in
    /// a std::set.
    template<>
    struct less<fostlib::email_address> :
    public std::
            binary_function<bool, fostlib::email_address, fostlib::email_address> {
        /// Answer the collation question
        bool operator()(
                const fostlib::email_address &l,
                const fostlib::email_address &r) const {
            return less<fostlib::rfc822_address>()(l.email(), r.email());
        }
    };
}


#endif // FOST_INTERNET_SMTP_HPP
