/*
    Copyright 1999-2014, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/url.hpp>
#include <fost/string>

#include <fost/exception/not_null.hpp>


using namespace fostlib;


namespace {


    template< typename C >
    C digit( utf8 dig ) {
        if ( dig < 0x0a ) return dig + '0';
        if ( dig < 0x10 ) return dig + 'A' - 0x0a;
        throw fostlib::exceptions::out_of_range< int >(
            L"Number to convert to hex digit is too big", 0, 0x10, dig
        );
    }
    template< typename S >
    S hex( utf8 ch ) {
        typename S::value_type num[ 4 ];
        num[ 0 ] = '%';
        num[ 1 ] = digit< typename S::value_type >( ( ch & 0xf0 ) >> 4 );
        num[ 2 ] = digit< typename S::value_type >( ch & 0x0f );
        num[ 3 ] = 0;
        return S( num );
    }


    const fostlib::utf8_string g_query_string_allowed(
        ".,:/\\_-*~="
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    );


}


/*
    fostlib::url::query_string
*/


fostlib::url::query_string::query_string() {
}
fostlib::url::query_string::query_string( const ascii_printable_string &q )
: m_string( q ) {
}


void fostlib::url::query_string::append(
    const string &name, const nullable< string > &value
) {
    if ( !m_string.isnull() ) {
        throw exceptions::not_null(
            "A plain text query string has already been provided");
    }
    m_query[ name ].push_back( value );
}


void fostlib::url::query_string::remove( const string &name ) {
    if ( !m_string.isnull() ) {
        throw exceptions::not_null(
            "A plain text query string has already been provided");
    }
    std::map< string, std::list< nullable< string > > >::iterator p(
        m_query.find( name ));
    if ( p != m_query.end() )
        m_query.erase( p );
}


std::size_t fostlib::url::query_string::has_key(const string &key) const {
    std::map< string, std::list< nullable< string > > >::const_iterator p(
        m_query.find(key));
    if ( p == m_query.end() ) {
        return 0;
    } else {
        return p->second.size();
    }
}


nullable< string > fostlib::url::query_string::operator [] (const string &k) const {
    std::map< string, std::list< nullable< string > > >::const_iterator p(
        m_query.find(k));
    if ( p == m_query.end() || p->second.size() == 0u ) {
        return null;
    } else {
        return *p->second.begin();
    }
}


namespace {
    const std::list< nullable< string > > c_empty_list;
}
const std::list< nullable< string > > &fostlib::url::query_string::at
        (const string &key) const {
    std::map< string, std::list< nullable< string > > >::const_iterator p(
        m_query.find(key));
    if ( p == m_query.end() ) {
        return c_empty_list;
    } else {
        return p->second;
    }
}


namespace {
    ascii_printable_string query_string_encode( const string &s ) {
        ascii_printable_string r; utf8_string i( coerce< utf8_string >( s ) );
        for ( utf8_string::const_iterator c( i.begin() ); c != i.end(); ++c )
            if ( g_query_string_allowed.underlying().find( *c ) == std::string::npos )
                r += hex< ascii_printable_string >( *c );
            else
                r += *c;
        return r;
    }
    nullable< ascii_printable_string > query_string_encode(
        const nullable< string > &s
    ) {
        if ( s.isnull() ) {
            return null;
        } else {
            return query_string_encode( s.value() );
        }
    }
}
nullable< ascii_printable_string > fostlib::url::query_string::as_string() const {
    if ( m_string.isnull() ) {
        nullable< ascii_printable_string > r;
        for (std::map< string, std::list< nullable< string > > >::const_iterator it(
                    m_query.begin());
                it != m_query.end(); ++it ) {
            for (
                std::list< nullable< string > >::const_iterator v( it->second.begin() );
                    v != it->second.end(); ++v) {
                r = concat(
                    r, ascii_printable_string( "&" ), concat(
                        query_string_encode( it->first ) + ascii_printable_string( "=" ),
                        query_string_encode( *v )));
            }
        }
        return r;
    } else {
        return m_string.value();
    }
}

