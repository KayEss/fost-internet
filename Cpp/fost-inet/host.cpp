/*
    Copyright 1999-2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/host.hpp>
#include <fost/parse/host.hpp>

#include <algorithm>
#include <cctype>


using namespace fostlib;


namespace {


    fostlib::string normalise(const fostlib::string &text) {
        fostlib::string ret;
        for ( fostlib::string::const_iterator c(text.begin()); c != text.end(); ++c )
            ret += std::tolower(*c);
        return ret;
    }


    fostlib::string ipname(uint32_t ipv4) {
        return coerce<string>(
            boost::asio::ip::address{
                boost::asio::ip::address_v4{ipv4}});
    }


}


fostlib::host::host() {
}
fostlib::host::host( const fostlib::string &name )
: m_name( normalise(name) ) {
}
fostlib::host::host(
    const fostlib::string &name, const nullable< string > &service
) : service( service ), m_name( normalise(name) ) {
}
fostlib::host::host( const fostlib::string &name, port_number service )
: service( coerce< string >(service) ), m_name( normalise(name) ) {
}

fostlib::host::host( uint32_t address, const nullable< string > &service )
: service(service), m_name(ipname(address)) {
}
fostlib::host::host( uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, const nullable< string > &service )
: service(service), m_name(ipname((b1 << 24) + (b2 << 16) + (b3 << 8) + b4 )) {
}


boost::asio::ip::address fostlib::host::address() const {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver( io_service );
    boost::asio::ip::tcp::resolver::query query(
        coerce<ascii_string>(name()).underlying(),
        coerce<ascii_string>(service().value_or("0")).underlying());
    boost::system::error_code error;
    boost::asio::ip::tcp::resolver::iterator it{resolver.resolve(query, error)};
    if ( error == boost::asio::error::host_not_found )
        throw exceptions::host_not_found(name());
    return it->endpoint().address();
}


string fostlib::host::name() const {
    return m_name;
}


host fostlib::coercer< host, string >::coerce(const string &hostname) {
    host r;
    auto begin = hostname.begin();
    if ( fostlib::host_p(begin, hostname.end(), r) ) {
        return r;
    } else {
        throw exceptions::not_implemented(__func__, "Where the host name didn't parse", hostname);
    }
}
string fostlib::coercer< string, boost::asio::ip::address >::coerce( const boost::asio::ip::address &address ) {
    return fostlib::coerce< string >( fostlib::coerce< ascii_string >( address.to_string() ) );
}
ascii_string fostlib::coercer<ascii_string, host>::coerce( const host &h ) {
    if ( not h.service() ) return fostlib::coerce< ascii_string >(h.name());
    else return fostlib::coerce< ascii_string >(h.name())
            + ascii_string(":")
            + fostlib::coerce< ascii_string >(h.service().value());
}


fostlib::exceptions::host_not_found::host_not_found( const string &hostname ) throw ()
: exception( L"Hostname: " + hostname ) {
}
const wchar_t * const fostlib::exceptions::host_not_found::message() const throw () {
    return L"Could not find an IP address for the host name";
}

