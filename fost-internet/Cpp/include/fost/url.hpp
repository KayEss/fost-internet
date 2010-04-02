/*
    Copyright 1999-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_URL_HPP
#define FOST_URL_HPP
#pragma once


#include <fost/unicode>
#include <fost/host.hpp>
#include <fost/detail/tagged-string.hpp>


namespace fostlib {


    class FOST_INET_DECLSPEC url {
    public:
        struct FOST_INET_DECLSPEC filepath_string_tag {
            static void do_encode( fostlib::nliteral from, ascii_printable_string &into );
            static void do_encode( const ascii_printable_string &from, ascii_printable_string &into, const bool encode_slash = false );
            static void check_encoded( const ascii_printable_string &s );
        };
        typedef tagged_string< filepath_string_tag, ascii_printable_string > filepath_string;

        class FOST_INET_DECLSPEC query_string {
        public:
            query_string();
            query_string( const string & );

            void append( const string &name, const nullable< string > &value );
            void remove( const string &name );

            nullable< ascii_printable_string > as_string() const;

        private:
            std::map< string, std::list< nullable< string > > > m_query;
        };
        enum t_form { e_pathname, e_encoded };

        url();
        explicit url( const string & );
        url( const url &base, const filepath_string &new_path );
        url( const url &base, const boost::filesystem::wpath &new_path );
        url( const t_form, const string & );
        explicit url( const host &,
            const nullable< string > &username = null,
            const nullable< string > &password = null
        );
        url( const ascii_printable_string &protocol, const host &,
            const nullable< string > &username = null,
            const nullable< string > &password = null
        );

        accessors< ascii_printable_string > protocol;
        accessors< host > server;
        port_number port() const;
        accessors< nullable< string > > user;
        accessors< nullable< string > > password;
        const filepath_string &pathspec() const;
        void pathspec( const filepath_string &pathName );
        accessors< nullable< ascii_printable_string > > anchor;
        accessors< query_string, fostlib::lvalue > query;

        static setting< string > s_default_host;

        ascii_printable_string as_string() const;
        ascii_printable_string as_string( const url &relative_from ) const;

    private:
        filepath_string m_pathspec;
    };


    namespace exceptions {


        class FOST_INET_DECLSPEC relative_path_error : public exception {
        public:
            relative_path_error( const string &base, const string &rel, const string &error ) throw ();

        protected:
            wliteral const message() const throw ();
        };


    }


    template<>
    struct FOST_INET_DECLSPEC coercer< string, url > {
        string coerce( const url &u ) {
            return fostlib::coerce< string >( u.as_string() );
        }
    };
    template<>
    struct FOST_INET_DECLSPEC coercer< url, string > {
        url coerce( const string &u ) {
            return url( u );
        }
    };

    template<>
    struct FOST_INET_DECLSPEC coercer< json, url > {
        json coerce( const url &u ) {
            return json( fostlib::coerce< string >( u.as_string() ) );
        }
    };
    template<>
    struct FOST_INET_DECLSPEC coercer< url, json > {
        url coerce( const json &u ) {
            return url( fostlib::coerce< string >( u ) );
        }
    };

    template<>
    struct FOST_INET_DECLSPEC coercer< url::filepath_string, string > {
        url::filepath_string coerce( const string &s );
    };
    template<>
    struct FOST_INET_DECLSPEC coercer< url::filepath_string, boost::filesystem::wpath > {
        url::filepath_string coerce( const boost::filesystem::wpath &s );
    };


}


namespace std {


    inline fostlib::ostream &operator << ( fostlib::ostream &o, const fostlib::url &u ) {
        return o << fostlib::coerce< fostlib::string >( u );
    }


}


#endif // FOST_URL_HPP

