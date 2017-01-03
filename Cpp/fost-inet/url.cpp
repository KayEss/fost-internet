/*
    Copyright 1999-2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/insert>
#include <fost/string>
#include <fost/url.hpp>
#include <fost/parse/url.hpp>

#include <fost/exception/out_of_range.hpp>
#include <fost/exception/parse_error.hpp>


using namespace fostlib;


namespace {


    const setting< bool > g_allow_relative(
        L"fost-base/Cpp/fost-inet/url.cpp", L"url", L"Allow relative urls", true, true
    );


    template< typename C >
    C digit( utf8 dig ) {
        if ( dig < 0x0a ) return dig + '0';
        if ( dig < 0x10 ) return dig + 'A' - 0x0a;
        throw fostlib::exceptions::out_of_range< int >(
            L"Number to convert to hex digit is too big", 0, 0x10, dig);
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

    unsigned char undigit(char d) {
        if ( d >= '0' && d <= '9' )
            return d - '0';
        else if ( d >= 'A' && d <= 'F' )
            return d - 'A' + 0xa;
        else if ( d >= 'a' && d <= 'f' )
            return d - 'a' + 0xa;
        throw fostlib::exceptions::out_of_range< int >(
            "Hex digit is not valid", '0', 'F', d);
    }
    unsigned char unhex(char c1, char c2) {
        return (undigit(c1) << 4) + undigit(c2);
    }

    const fostlib::utf8_string g_url_allowed(
        ".,:/\\_-~!"
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    const fostlib::utf8_string g_url_allowed_lax(
        ".,:/\\_-@&*+~!()[]'"
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    const fostlib::utf8_string g_url_part_allowed(
        // Slightly safer without the backslash and / : ~
        ".,_-!"
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ");


    /*
     * Note that given a pathSpec ending in '/', this will leave it like that.
     * What actually happens is that a blank entry gets shoved into the list,
     * and this recreates it later.
     * All good stuff, and exactly what we want.
     */
    std::string normalise_path( std::string t_path ) {
        auto slashslash = [&]() {
                while( t_path.find( "//" ) != std::string::npos )
                    t_path.replace( t_path.find( "//" ), 2, "/" );
            };
        slashslash();

        if( t_path[0] == '/' )
            // Trim off any leading / - otherwise we get a blank entry at the beginning
            t_path = t_path.substr( 1 );

        typedef std::list< std::string > pathlist_type;
        pathlist_type pathlist;

        for( int i=0;; i++ ) {
            pathlist.push_back( t_path.substr(0, t_path.find_first_of('/')) );
            if( std::string::npos == t_path.find_first_of( '/' ) )
                break;
            t_path=t_path.substr( t_path.find_first_of( '/' ) + 1 );
        }
        for( pathlist_type::iterator P=pathlist.begin(); P!=pathlist.end(); ++P ) {
            if( (*P)=="." ) {
                pathlist.erase( P );
                P=pathlist.begin();
            } else if( (*P)==".." ) {
                if( P==pathlist.begin() ) {
                    pathlist.erase( P );
                } else {
                    pathlist_type::iterator P1=--P;
                    ++P;
                    pathlist_type::iterator P2=++P;
                    pathlist.erase( P1, P2 );
                }
                P=pathlist.begin();
            }
        }
        t_path = "";
        for( pathlist_type::const_iterator P=pathlist.begin(); P!=pathlist.end(); ++P ) {
            t_path += '/' + (*P);
        }
        slashslash();
        return t_path;
    }
    url::filepath_string normalise_path( const url::filepath_string &path ) {
        return url::filepath_string(
            ascii_printable_string( normalise_path( path.underlying().underlying() ) )
        );
    }


}


/*
    fostlib::url::filepath_string_tag
*/


void fostlib::url::filepath_string_tag::do_encode(
    fostlib::nliteral from, ascii_printable_string &into
) {
    throw fostlib::exceptions::not_implemented( L"fostlib::url::filepath_string_tag::do_encode("
    L"fostlib::nliteral from, ascii_printable_string &into )" );
}


void fostlib::url::filepath_string_tag::do_encode(
    const ascii_printable_string &from, ascii_printable_string &into,
    const bool encode_slash
) {
    into.clear();
    std::size_t length = from.underlying().length();
    into.reserve( length + length / 2);
    for (
        ascii_printable_string::const_iterator i(from.begin()); i != from.end(); ++i
    )
        if ( (encode_slash ? g_url_part_allowed : g_url_allowed)
                .underlying().find( *i ) == std::string::npos )
            into += hex< ascii_printable_string >(*i);
        else
            into += *i;
}


void fostlib::url::filepath_string_tag::check_encoded(
    const ascii_printable_string &s
) {
    for ( ascii_printable_string::const_iterator c( s.begin() ); c != s.end(); ++c )
        if ( g_url_allowed_lax.underlying().find( *c ) == std::string::npos ) {
            if ( *c == '%' )
                for ( std::size_t p = 0; p != 2; ++p ) {
                    if ( ++c == s.end() )
                        throw fostlib::exceptions::parse_error(
                            L"File specification escape sequence is truncated"
                        );
                    if (
                        ( *c >= '0' && *c <= '9' )
                        || ( *c >= 'a' && *c <= 'f' )
                        || ( *c >= 'A' && *c <= 'F' )
                    ) ; // This is fine
                    else
                        throw fostlib::exceptions::parse_error(
                            L"File specification contains an illegal character in"
                            L"an escape sequence", string( 1, *c )
                        );
                }
            else
                throw fostlib::exceptions::parse_error(
                    L"File specification contains an illegal character", string( 1, *c )
                );
        }
}


url::filepath_string fostlib::coercer<
    url::filepath_string, string
>::coerce( const string &str ) {
    utf8_string narrowed( fostlib::coerce< utf8_string >( str ) );
    url::filepath_string encoded;
    for (
        utf8_string::const_iterator it( narrowed.begin() );
        it != narrowed.end(); ++it
    )
        if ( g_url_allowed.underlying().find( *it ) == std::string::npos )
            encoded += hex< url::filepath_string >( *it );
        else
            encoded += *it;
    return encoded;
}


url::filepath_string fostlib::coercer<
    url::filepath_string, boost::filesystem::wpath
>::coerce( const boost::filesystem::wpath &p ) {
    return fostlib::coerce< url::filepath_string >(
        fostlib::coerce< string >(p));
}


boost::filesystem::wpath fostlib::coercer<
    boost::filesystem::wpath, url::filepath_string
>::coerce( const url::filepath_string &s ) {
    utf8_string narrowed;
    for ( ascii_printable_string::const_iterator p(s.underlying().begin()); p != s.underlying().end(); ++p ) {
        if ( *p == '%' ) {
            char d1 = *++p;
            char d2 = *++p;
            narrowed += unhex(d1, d2);
        } else
            narrowed += *p;
    }
#if BOOST_FILESYSTEM_VERSION >= 3
    return boost::filesystem::wpath(narrowed.underlying());
#else
    return fostlib::coerce<boost::filesystem::wpath>(
        fostlib::coerce<string>(narrowed));
#endif
}


/*
    fostlib::url
*/


setting< string > fostlib::url::s_default_host(
    L"fost-base/Cpp/fost-inet/url.cpp",
    L"Site", L"Host", L"localhost", true
);


fostlib::url::url()
: protocol( "http" ), server( host(s_default_host.value()) ), m_pathspec( "/" ) {
}
fostlib::url::url( const url& url, const char *path )
: protocol( url.protocol() ), server( url.server() ), m_pathspec( "/" ) {
    pathspec( ascii_printable_string(path) );
}
fostlib::url::url( const url& url, const filepath_string &path )
: protocol( url.protocol() ), server( url.server() ), m_pathspec( "/" ) {
    pathspec( path );
}
fostlib::url::url( const url& url, const boost::filesystem::wpath &path )
: protocol( url.protocol() ), server( url.server() ), m_pathspec( "/" ) {
    pathspec( coerce< filepath_string >( path ) );
}
fostlib::url::url( const t_form form, const string &str )
: protocol( "http" ), server( host(s_default_host.value()) ), m_pathspec( "/" ) {
    std::pair< string, nullable< string > > anchor_parts( partition( str, L"#" ) );
    std::pair< string, nullable< string > > query_parts(
        partition( anchor_parts.first, L"?" )
    );
    switch ( form ) {
    case e_pathname:
        m_pathspec = url::filepath_string( ascii_printable_string(
            normalise_path(
                coerce< url::filepath_string >( str ).underlying().underlying()
            )
        ) );
        break;
    case e_encoded:
        for ( string::const_iterator it( str.begin() ); it != str.end(); ++it )
            if ( *it < 0x20 || *it > 0x7f )
                throw fostlib::exceptions::parse_error(
                    L"The encoded url contains an invalid character (" + str + L")"
                );
        m_pathspec = coerce< url::filepath_string >( query_parts.first );
        break;
    }
    if ( query_parts.second )
        query( query_string( coerce< ascii_printable_string >(
            query_parts.second.value()
        ) ) );
    if ( anchor_parts.second )
        anchor( coerce< ascii_printable_string >( anchor_parts.second.value() ) );
}
fostlib::url::url(
    const fostlib::host &h, const nullable< string > &u, const nullable< string > &pw
) : protocol( "http" ), server( h ), user( u ), password( pw ), m_pathspec( "/" ) {
}
fostlib::url::url(const string &a_url)
: protocol("http"), server(host(s_default_host.value())), m_pathspec("/") {
    try {
        parser_lock lock;
        auto pos = a_url.begin();
        if ( not url_p(lock, pos, a_url.end(), *this) || pos != a_url.end() ) {
            throw exceptions::parse_error("Could not parse URL string", string(pos, a_url.end()));
        }
    } catch ( exceptions::exception &e ) {
        insert(e.data(), "parsing", a_url);
        throw;
    }
}
fostlib::url::url( const ascii_printable_string &protocol, const host &h,
    const nullable< string > &username,
    const nullable< string > &password
) : protocol( protocol ), server( h ),
        user( username ), password( password ), m_pathspec( "/" ) {
}

ascii_printable_string fostlib::url::as_string() const {
    ascii_printable_string url( protocol() + ascii_printable_string( "://" ) );
    if ( user() )
        url += coerce< ascii_printable_string >(
            user().value() + L":" + password().value_or( string() ) + L"@"
        );
    else if ( password() )
        url += coerce< ascii_printable_string >( L":" + password().value() + L"@" );
    url += coerce< ascii_printable_string >(server().name());
    if ( server().service() && (
            ( protocol() == ascii_printable_string("http")
                && server().service() != "80" )
            || ( protocol() == ascii_printable_string("https")
                && server().service() != L"443" )
    ) ) url += coerce< ascii_printable_string >(L":" + server().service().value());
    url += pathspec().underlying();
    url = concat( url, ascii_printable_string( "?" ), query().as_string() ).value();
    return concat( url, ascii_printable_string( "#" ), anchor() ).value();
}

ascii_printable_string fostlib::url::as_string( const url &relative_from ) const {
    if ( g_allow_relative.value() &&
        (
            protocol() == ascii_printable_string( "http" )
            || protocol() == ascii_printable_string( "https" )
        ) && relative_from.server().service() == server().service()
        && relative_from.server().name() == server().name()
    )
        return concat(
            concat(
                m_pathspec.underlying(),
                ascii_printable_string( "?" ),
                query().as_string()
            ),
            ascii_printable_string( "#" ),
            anchor()
        ).value();
    else
        return as_string();
}

fostlib::port_number fostlib::url::port() const {
    if ( not server().service() )
        return protocol() == ascii_printable_string("http") ? 80 : 443;
    else
        return coerce< port_number >( server().service().value() );
}

const url::filepath_string &fostlib::url::pathspec() const {
    return m_pathspec;
}

void fostlib::url::pathspec( const url::filepath_string &a_pathName ) {
    url::filepath_string pathName( a_pathName );
    // First, formalise the new path.
    if( pathName.underlying().underlying().length() == 0 ) {
        // Assume that if it's blank, the caller means '/'
        pathName = url::filepath_string( "/" );
    }
    // Obvious directory fixes.
    if( pathName.underlying().underlying().find( "/." )
            ==(pathName.underlying().underlying().length()-2) ) {
        pathName += '/'; // Add terminating slash if it ends with /.
    } else if( pathName.underlying().underlying().find( "/.." )
            ==(pathName.underlying().underlying().length()-3) ) {
        pathName += url::filepath_string( "/" ); // Or /..
    } else if( pathName==url::filepath_string( "." ) ) {
        pathName += url::filepath_string( "/" );
    } else if( pathName==url::filepath_string( ".." ) ) {
        // Or if it's simply '..' or '.', both of which really mean '../' or './' anyway.
        pathName += url::filepath_string( "/" );
    }
    // Now do we add or replace?
    if ( pathName.underlying().underlying()[ 0 ] != '/' ) {
        if( m_pathspec.underlying().underlying()[
                m_pathspec.underlying().underlying().length() - 1
        ] != '/' )
            m_pathspec += url::filepath_string( "/../" );
        // Whatever happens, we don't need the last part of the
        // pathSpec anymore - the file name.
        m_pathspec = normalise_path( m_pathspec + pathName );
    } else // It begins with a /, so we replace.
        m_pathspec = normalise_path( pathName );
}


/*
    fostlib::exceptions::relative_path_error
*/


fostlib::exceptions::relative_path_error::relative_path_error(
    const string &base, const string &rel, const string &error
) throw ()
: exceptions::exception(error) {
    try {
        insert(data(), "base", base);
        insert(data(), "relative", rel);
    } catch ( ... ) {
        absorb_exception();
    }
}
wliteral const fostlib::exceptions::relative_path_error::message() const throw () {
    return L"Relative path error";
}

