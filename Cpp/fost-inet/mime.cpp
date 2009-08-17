/*
    Copyright 1999-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/mime.hpp>
#include <fost/string>
#include <fost/exception/parse_error.hpp>


using namespace fostlib;


/*
    fostlib::mime
*/


fostlib::mime::mime( const string &content_type )
: content_type( content_type ) {
}

fostlib::mime::mime( const mime_headers &h, const string &content_type )
: content_type( content_type ), headers( h ) {
}

fostlib::mime::~mime() {
}

bool fostlib::mime::boundary_is_ok( const string &/*boundary*/ ) const {
    return true;
}

std::ostream &fostlib::mime::print_on( std::ostream &o ) const {
    string boundary;
    do {
        boundary = guid();
    } while ( !boundary_is_ok( boundary ) );

    o << headers() << "Content-Type: " << coerce< utf8string >( content_type() ) << "; boundary=" + coerce< utf8string >( boundary ) + "\r\n";
    for ( std::list< boost::shared_ptr< mime > >::const_iterator part( items().begin() ) ;part != items().end(); ++part ) {
        o << "\r\n--" << coerce< utf8string >( boundary ) << "\r\n";
        (*part)->print_on( o );
    }
    return o << "\r\n--" << coerce< utf8string >( boundary ) << "--\r\n";
}


/*
    fostlib::mime::mime_headers
*/


std::pair< string, headers_base::content > fostlib::mime::mime_headers::value( const string &name, const string &value ) {
    if ( name == L"Content-Disposition" || name == L"content-disposition" || name == L"Content-Type" ) {
        std::map< string, string > args;
        // Parse the value from the format
        // form-data; name="aname"; extra="value"
        std::pair< string, nullable< string > > disp( partition( value, L";" ) );
        if ( !disp.second.isnull() ) {
            for ( std::pair< string, nullable< string > > para( partition( disp.second, L";" ) ); !para.first.empty(); para = partition( para.second, L";" ) ) {
                // Normally the extra argument values should be surrounded by double quotes, but sometimes not
                std::pair< string, nullable< string > > argument( crack( para.first, L"=\"", L"\"" ) );
                if ( argument.second.isnull() )
                    argument = partition( para.first, L"=" );

                if ( argument.second.isnull() )
                    throw exceptions::parse_error( L"Message header " + name + L" does not have a value for an argument", para.first );
                args[ argument.first ] = argument.second.value();
            }
        }
        if ( name == L"Content-Disposition" ) // RFC 1867 uses lower case
            return std::make_pair( L"content-disposition", content( disp.first, args ) );
        else
            return std::make_pair( name, content( disp.first, args ) );
    } else
        return std::make_pair( name, content( value ) );
}


/*
    fostlib::text_body
*/

namespace {
    void do_headers(text_body &tb, const utf8string &body, const string &mime_type) {
        tb.headers().add(L"Content-Type", mime::mime_headers::content(mime_type).subvalue( L"charset", L"utf-8" ));
        tb.headers().add(L"Content-Transfer-Encoding", L"8bit");
        tb.headers().add(L"Content-Length", coerce< string >(body.length()));
    }
}
fostlib::text_body::text_body(const utf8 *begin, const utf8 *end, const string &mime_type)
: text(utf8string(begin, end)) {
    do_headers(*this, text(), mime_type);
}
fostlib::text_body::text_body(const utf8string &t, const string &mime_type)
: text(t) {
    do_headers(*this, text(), mime_type);
}
fostlib::text_body::text_body(const fostlib::string &t, const fostlib::string &mime_type)
: text(coerce< utf8string >(t)) {
    do_headers(*this, text(), mime_type);
}

std::ostream &fostlib::text_body::print_on( std::ostream &o ) const {
    return o << headers() << "\r\n" << text();
}

bool fostlib::text_body::boundary_is_ok( const string &boundary ) const {
    return text().find( coerce< utf8string >( boundary ) ) == string::npos;
}
