/*
    Copyright 1999-2015, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/connection.hpp>
#include <fost/smtp.hpp>
#include <fost/parse/parse.hpp>

#include <fost/exception/null.hpp>
#include <fost/exception/parse_error.hpp>

#include <fost/insert>


using namespace fostlib;


/*
    fostlib::rfc822_address_tag
*/


void fostlib::rfc822_address_tag::do_encode( fostlib::nliteral from, ascii_printable_string &into ) {
    throw exceptions::not_implemented("fostlib::rfc822_address_tag::do_encode( fostlib::nliteral from, ascii_string &into )");
}
void fostlib::rfc822_address_tag::do_encode( const ascii_printable_string &from, ascii_printable_string &into ) {
    throw exceptions::not_implemented("fostlib::rfc822_address_tag::do_encode( const ascii_string &from, ascii_string &into )");
}
void fostlib::rfc822_address_tag::check_encoded( const ascii_printable_string &s ) {
    if ( s.empty() )
        throw exceptions::null( L"Email address is empty" );
    if ( s.underlying().find( '@' ) == string::npos )
        throw exceptions::parse_error( L"Email address doesn't contain @ symbol", coerce< string >(s) );
}


/*
    fostlib::email_address
*/


fostlib::email_address::email_address() {
}

fostlib::email_address::email_address( const rfc822_address &address, const nullable<string> &name )
: email( address ), name( name ) {
}

fostlib::email_address::email_address(const ascii_printable_string &address, const nullable< string > &name)
: email(rfc822_address(address)),
    name(name) {
}


string fostlib::coercer< string, email_address >::coerce( const email_address &e ) {
    if ( e.name().isnull() )
        return L"<" + fostlib::coerce< string >( e.email().underlying() ) + ">";
    else
        return e.name().value() + L" <" + fostlib::coerce< string >( e.email().underlying() ) + L">";
}
email_address fostlib::coercer< email_address, string >::coerce( const string &s ) {
    fostlib::parser_lock lock;
    string name, address1, address2;
    if ( !fostlib::parse(lock, s.c_str(),
        ((+boost::spirit::chset< wchar_t >(L"a-zA-Z0-9_\\.\\+ -"))[
            phoenix::var( name ) = phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 )
        ] >> boost::spirit::chlit< wchar_t >( '<' )
            >> ((+boost::spirit::chset< wchar_t >(L"a-zA-Z0-9_@\\.\\+-" ))[
                phoenix::var( address1 ) = phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 )
            ])
            >> boost::spirit::chlit< wchar_t >( '>' ))
        | ((+boost::spirit::chset< wchar_t >(L"a-zA-Z0-9_@\\.\\+-" ))[
            phoenix::var( address2 ) = phoenix::construct_< string >( phoenix::arg1, phoenix::arg2 )
        ])
    ).full )
        throw exceptions::not_implemented("fostlib::coercer< email_address, string >::coerce( const string &s ) -- could not parse", s);
    if ( address1.empty() )
        return rfc822_address( fostlib::coerce< ascii_printable_string >( address2 ) );
    else
        return email_address( rfc822_address( fostlib::coerce< ascii_printable_string >( address1 ) ),
            trim(name) );
}


/*
    fostlib::smtp_client
*/


struct fostlib::smtp_client::implementation {
    bool can_send;
    network_connection cnx;

    implementation(const host &h, const port_number p)
    : can_send(false), cnx(h, p) {
        check(220, L"Initial connection");
        cnx << "HELO FSIP\r\n";
        check(250, L"HELO");
        can_send = true;
    }
    ~implementation() {
        if ( can_send ) {
            cnx << "QUIT\r\n";
            check(221, L"QUIT");
        }
    }

    void check( int code, const string &command ) {
        utf8_string number = coerce< utf8_string >(
            coerce< string >( code ));
        utf8_string response;
        cnx >> response;
        if ( response.underlying().substr( 0, number.underlying().length() ) != number.underlying() ) {
            exceptions::not_implemented exception(L"SMTP response was not the one expected");
            insert(exception.data(), "code", "expected", code);
            insert(exception.data(), "code", "received",
                response.underlying().substr(0, number.underlying().length()).c_str());
            insert(exception.data(), "command", command);
            insert(exception.data(), "response", response);
            throw exception;
        }
    }
};


fostlib::smtp_client::smtp_client(const host &server, const port_number p)
: m_impl(new implementation(server, p)) {
}

fostlib::smtp_client::~smtp_client()
try {
    delete m_impl;
} catch ( ... ) {
    absorb_exception();
}


void fostlib::smtp_client::send(const mime &email,
    const rfc822_address &to, const rfc822_address &from
) {
    m_impl->can_send = false;

    m_impl->cnx << "MAIL FROM:<" + from.underlying().underlying() + ">\r\n";
    m_impl->check(250, L"MAIL FROM");
    m_impl->cnx << "RCPT TO:<" + to.underlying().underlying() + ">\r\n";
    m_impl->check(250, L"RCPT TO");
    m_impl->cnx << "DATA\r\n";
    m_impl->check(354, L"DATA");

    std::stringstream ss;
    ss << email.headers();
    ss << "\r\n";
    m_impl->cnx << ss;

    for ( mime::const_iterator d( email.begin() ); d != email.end(); ++d )
        m_impl->cnx << *d;
    m_impl->cnx << "\r\n.\r\n";
    m_impl->check(250, L"Data spooling");

    m_impl->can_send = true;
}

