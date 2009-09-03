/*
    Copyright 1999-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_MIME_HPP
#define FOST_INTERNET_MIME_HPP
#pragma once


#include <fost/core>
#include <fost/pointers>
#include <boost/filesystem.hpp>


namespace fostlib {


    class FOST_INET_DECLSPEC FSL_ABSTRACT headers_base {
    public:
        class content;

        headers_base();
        virtual ~headers_base();

        void parse( const string &headers );

        bool exists( const string & ) const;
        content &set( const string &name, const content & );
        const content &operator [] ( const string & ) const;

        typedef std::map< string, content >::const_iterator const_iterator;
        const_iterator begin() const;
        const_iterator end() const;

        class FOST_INET_DECLSPEC content {
        public:
            content();
            content( wliteral );
            content( const string & );
            content( const string &, const std::map< string, string > & );

            accessors< string > value;

            content &subvalue( const string &k, const string &v );
            nullable< string > subvalue( const string &k ) const;

            typedef std::map< string, string >::const_iterator const_iterator;
            const_iterator begin() const;
            const_iterator end() const;

        private:
            std::map< string, string > m_subvalues;
        };

    protected:
        virtual std::pair< string, content > value( const string &name, const string &value ) = 0;

    private:
        std::map< string, content > m_headers;
    };


    /*
        This initial implementation is only intended to be able to parse HTTP POST multipart/form-data encoded data and HTTP host headers. A later implementation will need to consider the transport parameters in order to know how to encode the content and which headers are required.
    */
    class FOST_INET_DECLSPEC mime : boost::noncopyable {
        protected:
            struct FOST_INET_DECLSPEC iterator_implementation {
                virtual ~iterator_implementation();
                virtual const_memory_block operator () () = 0;
            };
        public:
            class FOST_INET_DECLSPEC mime_headers : public fostlib::headers_base {
                protected:
                    std::pair< string, headers_base::content > value( const string &name, const string &value );
            };
            virtual ~mime();

            accessors< string > content_type;
            accessors< mime_headers, fostlib::lvalue > headers;

            virtual bool boundary_is_ok( const string &boundary ) const = 0;
            virtual std::ostream &print_on( std::ostream & ) const = 0;

            class FOST_INET_DECLSPEC const_iterator {
                friend class fostlib::mime;
                boost::shared_ptr< iterator_implementation > underlying;
                const_memory_block current;
                const_iterator( std::auto_ptr< iterator_implementation > p );
                public:
                    bool operator == ( const const_iterator & ) const;
                    bool operator != ( const const_iterator & ) const;
                    const_memory_block operator * () const;
                    const const_iterator &operator ++();
            };

            const_iterator begin() const;
            const_iterator end() const;

        protected:
            virtual std::auto_ptr< iterator_implementation > iterator() const = 0;

            explicit mime( const mime_headers &headers, const string &content_type );
    };


    class FOST_INET_DECLSPEC empty_mime : public mime {
        struct empty_mime_iterator;
        std::auto_ptr< iterator_implementation > iterator() const;
        public:
            empty_mime(const mime_headers &headers = mime_headers());

            std::ostream &print_on( std::ostream &o ) const;
            bool boundary_is_ok( const string &boundary ) const;
    };

    class FOST_INET_DECLSPEC mime_envelope : public mime {
        std::auto_ptr< iterator_implementation > iterator() const;
        public:
            mime_envelope(const mime_headers &headers = mime_headers());

            std::ostream &print_on( std::ostream &o ) const;
            bool boundary_is_ok( const string &boundary ) const;

            accessors< std::list< boost::shared_ptr< mime > >, fostlib::lvalue > items;
    };

    class FOST_INET_DECLSPEC text_body : public mime {
        struct text_body_iterator;
        std::auto_ptr< iterator_implementation > iterator() const;
        public:
            text_body( const utf8 *begin, const utf8 *end, const mime_headers &headers = mime_headers(), const string &mime = "text/plain" );
            text_body( const utf8string &text, const mime_headers &headers = mime_headers(), const string &mime = "text/plain" );
            text_body( const string &text, const mime_headers &headers = mime_headers(), const string &mime = L"text/plain" );

            std::ostream &print_on( std::ostream &o ) const;
            bool boundary_is_ok( const string &boundary ) const;

            accessors< const utf8string > text;
    };

    class FOST_INET_DECLSPEC file_body : public mime {
        struct file_body_iteration;
        std::auto_ptr< iterator_implementation > iterator() const;
        public:
            file_body(
                const boost::filesystem::wpath &file,
                const mime_headers &headers = mime_headers(),
                const string &mime = "binary/octet-stream"
            );

            std::ostream &print_on( std::ostream &o ) const;
            bool boundary_is_ok( const string &boundary ) const;

            accessors< const boost::filesystem::wpath > filename;
    };


    FOST_INET_DECLSPEC std::ostream &operator <<( std::ostream &o, const headers_base &headers );
    FOST_INET_DECLSPEC std::ostream &operator <<( std::ostream &o, const headers_base::content &value );
    inline std::ostream &operator << ( std::ostream &o, const mime &m ) {
        return m.print_on( o );
    }
#ifdef WIN32
    inline std::wostream &operator << ( std::wostream &o, const mime &m ) {
        std::stringstream ss;
        ss << m;
        return o << coerce< string >( ss.str() );
    }
#endif


}


#endif // FOST_INTERNET_MIME_HPP
