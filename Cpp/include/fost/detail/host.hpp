/*
    Copyright 1999-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_HOST_HPP
#define FOST_HOST_HPP
#pragma once


#include <fost/string>
#include <boost/asio.hpp>


namespace fostlib {


    typedef uint16_t port_number;


    class FOST_INET_DECLSPEC host {
    public:
        host();
        explicit host( const string &host, const nullable< string > &service = null );
        explicit host( uint32_t ipv4, const nullable< string > &service = null );
        host( uint8_t, uint8_t, uint8_t, uint8_t, const nullable< string > &service = null  );

        boost::asio::ip::address address() const;
        string name() const;
        accessors< nullable< string > > service;

    private:
        fostlib::string m_name, m_service;
        mutable nullable< boost::asio::ip::address > m_address;
    };


    namespace exceptions {


        class FOST_INET_DECLSPEC host_not_found : public exception {
        public:
            host_not_found( const string &host_name ) throw ();

        protected:
            const wchar_t * const message() const throw ();
        };


    }


    template<>
    struct FOST_INET_DECLSPEC coercer< string, boost::asio::ip::address > {
        string coerce( const boost::asio::ip::address &i );
    };
    template<>
    struct FOST_INET_DECLSPEC coercer< ascii_string, host > {
        ascii_string coerce( const host &i );
    };


}


namespace std {


    inline fostlib::ostream  &operator <<( fostlib::ostream &o, const fostlib::host &h ) {
        return o << fostlib::coerce< fostlib::string >( fostlib::coerce< fostlib::ascii_string >( h ) );
    }


}


#endif // FOST_HOST_HPP

