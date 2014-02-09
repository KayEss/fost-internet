/*
    Copyright 1999-2011, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#ifndef FOST_INTERNET_MIME_HPP
#define FOST_INTERNET_MIME_HPP
#pragma once


#include <fost/headers.hpp>


namespace fostlib {


    /// An abstract base class for MIME containers
    class FOST_INET_DECLSPEC mime : boost::noncopyable {
        protected:
            struct FOST_INET_DECLSPEC iterator_implementation {
                virtual ~iterator_implementation();
                virtual const_memory_block operator () () = 0;
            };
        public:
            /// MIME headers
            class FOST_INET_DECLSPEC mime_headers :
                    public fostlib::headers_base {
                protected:
                    std::pair< string, headers_base::content > value(
                        const string &name, const string &value);
            };
            virtual ~mime();

            accessors< string > content_type;
            accessors< mime_headers, fostlib::lvalue > headers;

            virtual bool boundary_is_ok( const string &boundary ) const = 0;
            virtual std::ostream &print_on( std::ostream & ) const = 0;

            /// An iterator that allows MIME data to be traversed
            class FOST_INET_DECLSPEC const_iterator {
                friend class fostlib::mime;
                boost::shared_ptr< iterator_implementation > underlying;
                const_memory_block current;
                const_iterator( std::auto_ptr< iterator_implementation > p );
                public:
                    /// Allow comparison for equality
                    bool operator == ( const const_iterator & ) const;
                    /// Allow comparison for inequality
                    bool operator != ( const const_iterator & ) const;
                    /// Allow the memory block to be accessed
                    const_memory_block operator * () const;
                    /// Move to the next chunk of data
                    const const_iterator &operator ++ ();
            };

            /// Fetch the start of the MIME data
            const_iterator begin() const;
            /// Fetch the end of the MIME data
            const_iterator end() const;

        protected:
            virtual std::auto_ptr< iterator_implementation > iterator() const = 0;

            explicit mime(
                const mime_headers &headers, const string &content_type
            );
    };


    /// A MIME implementation that can never have any body data.
    class FOST_INET_DECLSPEC empty_mime : public mime {
        struct empty_mime_iterator;
        std::auto_ptr< iterator_implementation > iterator() const;
        public:
            /// Construct an empty MIME body that cannot have data
            empty_mime(const mime_headers &headers = mime_headers(),
                const string &mime = "application/x-empty");

            /// Print the MIME out on the stream
            std::ostream &print_on( std::ostream &o ) const;
            /// Check to see if the boundary is OK
            bool boundary_is_ok( const string &boundary ) const;
    };

    /// A MIME envelope can contain a nested structure of other MIME containers
    class FOST_INET_DECLSPEC mime_envelope : public mime {
        struct mime_envelope_iterator;
        std::auto_ptr< iterator_implementation > iterator() const;
        string m_boundary;
        public:
            /// Construct the MIME envelope with optional headers
            mime_envelope(const mime_headers &headers = mime_headers(),
                const string &mime = "multipart/mixed");

            /// Print the MIME out on the stream
            std::ostream &print_on( std::ostream &o ) const;
            /// Check that the boundary can be used
            bool boundary_is_ok( const string &boundary ) const;

            /// The type of the attachments
            typedef std::list< boost::shared_ptr< mime > > items_type;
            /// The embedded MIME items
            accessors< items_type, fostlib::lvalue > items;

            /// Attach a MIME type
            template< typename M, typename P1 >
            boost::shared_ptr< M > attach(const P1 &p1) {
                boost::shared_ptr< M > attachment( new M(p1) );
                items().push_back(attachment);
                return attachment;
            }
            /// Attach a MIME type
            template< typename M, typename P1, typename P2 >
            boost::shared_ptr< M > attach(const P1 &p1, const P2 &p2) {
                boost::shared_ptr< M > attachment( new M(p1, p2) );
                items().push_back(attachment);
                return attachment;
            }
            /// Attach a MIME type
            template< typename M, typename P1, typename P2, typename P3 >
            boost::shared_ptr< M > attach(const P1 &p1, const P2 &p2,
                    const P3 &p3) {
                boost::shared_ptr< M > attachment( new M(p1, p2, p3) );
                items().push_back(attachment);
                return attachment;
            }

            /// After all attachments have been added, use this to find a boundary
            const string &boundary();
    };

    /// A MIME container which always stores text
    class FOST_INET_DECLSPEC text_body : public mime {
        struct text_body_iterator;
        std::auto_ptr< iterator_implementation > iterator() const;
        public:
            text_body(
                const utf8 *begin,
                const utf8 *end,
                const mime_headers &headers = mime_headers(),
                const string &mime = "text/plain"
            );
            text_body(
                const utf8_string &text,
                const mime_headers &headers = mime_headers(),
                const string &mime = "text/plain"
            );
            text_body(
                const string &text,
                const mime_headers &headers = mime_headers(),
                const string &mime = L"text/plain"
            );

            /// Print the MIME out on the stream
            std::ostream &print_on( std::ostream &o ) const;
            /// Check that the boundary can be used
            bool boundary_is_ok( const string &boundary ) const;

            accessors< const utf8_string > text;
    };

    /// A MIME container which represents binary data in memory
    class FOST_INET_DECLSPEC binary_body : public mime {
        struct binary_body_iterator;
        std::auto_ptr< iterator_implementation > iterator() const;
        public:
            /// Construct an empty body
            binary_body(
                const mime_headers &headers = mime_headers(),
                const string &mime = "application/x-empty"
            );
            /// Construct from a data block
            binary_body(
                const std::vector< unsigned char > &data,
                const mime_headers &headers = mime_headers(),
                const string &mime = "binary/octet-stream"
            );

            /// Display the data on a stream
            std::ostream &print_on( std::ostream &o ) const;
            /// Check that the boundary can be used
            bool boundary_is_ok( const string &boundary ) const;

            /// Allow direct read access to the data
            accessors< const std::vector< unsigned char > > data;
    };

    /// A MIME container which represents a file on disk
    class FOST_INET_DECLSPEC file_body : public mime {
        struct file_body_iteration;
        std::auto_ptr< iterator_implementation > iterator() const;
        public:
            file_body(
                const boost::filesystem::wpath &file,
                const mime_headers &headers = mime_headers(),
                const string &mime = "binary/octet-stream"
            );

            /// Print the MIME out on the stream
            std::ostream &print_on( std::ostream &o ) const;
            /// Check that the boundary can be used
            bool boundary_is_ok( const string &boundary ) const;

            accessors< const boost::filesystem::wpath > filename;
    };


    /// Allow the UTF8 text of a MIME wrapper to be extracted.
    template<>
    struct FOST_INET_DECLSPEC coercer<utf8_string, mime> {
        /// Performs the coercion
        utf8_string coerce( const mime & );
    };

    /// Allow the conversion of the MIME content to be extracted as a string
    template<>
    struct FOST_INET_DECLSPEC coercer<string, mime> {
        /// Performs the coercion
        string coerce( const mime &m ) {
            return fostlib::coerce<fostlib::string>(
                fostlib::coerce<fostlib::utf8_string>(m));
        }
    };

    /// Allow the conversion of MIME subclasses
    template< typename T, typename F>
    struct coercer<T, F,
        typename boost::enable_if< boost::is_base_of<mime, F> >::type
    > {
        T coerce( const F &f ) {
            return fostlib::coerce<T, mime>( f );
        }
    };


    inline std::ostream &operator << ( std::ostream &o, const mime &m ) {
        return m.print_on( o );
    }
#ifdef WIN32
    inline std::wostream &operator << ( std::wostream &o, const mime &m ) {
        std::stringstream ss;
        ss << m;
        return o << coerce< string >( utf8_string( ss.str() ) );
    }
#endif


}


#endif // FOST_INTERNET_MIME_HPP
