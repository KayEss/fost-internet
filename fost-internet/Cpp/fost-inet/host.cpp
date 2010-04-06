/*
    Copyright 1999-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/host.hpp>
#include <fost/parse/host.hpp>


using namespace fostlib;


namespace {



    boost::asio::ip::address getaddr( const host &host ) {
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver( io_service );
        boost::asio::ip::tcp::resolver::query query(
            coerce< ascii_string >( host.name() ).underlying(),
            coerce< ascii_string >( host.service().value("0") ).underlying()
        );
        boost::system::error_code error;
        boost::asio::ip::tcp::resolver::iterator it( resolver.resolve( query, error ) );
        if ( error == boost::asio::error::host_not_found )
            throw exceptions::host_not_found( host.name() );
        return it->endpoint().address();
    }


}


fostlib::host::host() {
}


fostlib::host::host( const fostlib::string &name, const nullable< string > &service )
: service( service ), m_name( name ) {
}


fostlib::host::host( uint32_t address, const nullable< string > &service )
: service( service ), m_address( boost::asio::ip::address_v4( address ) ) {
    m_name = coerce< string >( m_address.value() );
}


fostlib::host::host( uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, const nullable< string > &service )
: service( service ), m_address(
    boost::asio::ip::address_v4( ( b1 << 24 ) + ( b2 << 16 ) + ( b3 << 8 ) + b4 )
) {
    m_name = coerce< string >( m_address.value() );
}


boost::asio::ip::address fostlib::host::address() const {
    if ( m_address.isnull() )
        m_address = getaddr( *this );
    return m_address.value();
}


string fostlib::host::name() const {
    return m_name;
}


host fostlib::coercer< host, string >::coerce( const string &h ) {
    host r;
    host_parser host_p;
    if ( boost::spirit::parse(h.c_str(), host_p[ phoenix::var(r) = phoenix::arg1 ]).full )
        return r;
    else
        throw exceptions::not_implemented(
            "fostlib::coercer< host, string >::coerce( const string &h )"
            " -- where the host name didn't parse"
        );
}
string fostlib::coercer< string, boost::asio::ip::address >::coerce( const boost::asio::ip::address &address ) {
    return fostlib::coerce< string >( fostlib::coerce< ascii_string >( address.to_string() ) );
}
ascii_string fostlib::coercer< ascii_string, host >::coerce( const host &h ) {
    if ( h.service().isnull() )
        return fostlib::coerce< ascii_string >( h.name() );
    else
        return fostlib::coerce< ascii_string >( h.name() )
            + ascii_string(":")
            + fostlib::coerce< ascii_string >( h.service().value() )
        ;
}


fostlib::exceptions::host_not_found::host_not_found( const string &hostname ) throw ()
: exception( L"Hostname: " + hostname ) {
}
const wchar_t * const fostlib::exceptions::host_not_found::message() const throw () {
    return L"Could not find an IP address for the host name";
}
