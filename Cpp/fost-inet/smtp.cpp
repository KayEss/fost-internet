/*
    Copyright 1999-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/smtp.hpp>
#include <fost/exception/not_implemented.hpp>
#include <fost/exception/null.hpp>
#include <fost/exception/parse_error.hpp>


using namespace fostlib;


/*
    fostlib::rfc822_address_tag
*/


void fostlib::rfc822_address_tag::do_encode( fostlib::nliteral from, ascii_string &into ) {
    throw exceptions::not_implemented("fostlib::rfc822_address_tag::do_encode( fostlib::nliteral from, ascii_string &into )");
}
void fostlib::rfc822_address_tag::do_encode( const ascii_string &from, ascii_string &into ) {
    throw exceptions::not_implemented("fostlib::rfc822_address_tag::do_encode( const ascii_string &from, ascii_string &into )");
}
void fostlib::rfc822_address_tag::check_encoded( const ascii_string &s ) {
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

fostlib::email_address::email_address( const rfc822_address &e, const nullable<string> &n )
: email( e ), name( n ) {
}
