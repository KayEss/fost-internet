/**
    Copyright 1999-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#ifndef FOST_INTERNET_MIME_HPP
#define FOST_INTERNET_MIME_HPP
#pragma once


#include <fost/headers.hpp>


namespace fostlib {


    /// An abstract base class for MIME containers
    class FOST_INET_DECLSPEC mime {
      protected:
        struct FOST_INET_DECLSPEC iterator_implementation {
            virtual ~iterator_implementation();
            virtual const_memory_block operator()() = 0;
        };

      public:
        /// MIME headers
        class FOST_INET_DECLSPEC mime_headers : public fostlib::headers_base {
          protected:
            std::pair<string, headers_base::content>
                    value(const string &name, const string &value);
        };
        virtual ~mime();

        /// Not copyable
        mime(mime const &) = delete;
        mime &operator=(mime const &) = delete;

        accessors<string> content_type;
        accessors<mime_headers, fostlib::lvalue> headers;

        virtual bool boundary_is_ok(const string &boundary) const = 0;
        virtual std::ostream &print_on(std::ostream &) const = 0;

        /// APIs for fetching the body as one of a these representations.
        /// These will throw if the body isn't valid or can't be parsed.
        virtual f5::u8string body_as_string() const;
        virtual fostlib::json body_as_json() const;

        /// An iterator that allows MIME data to be traversed
        class FOST_INET_DECLSPEC const_iterator {
            friend class fostlib::mime;
            boost::shared_ptr<iterator_implementation> underlying;
            const_memory_block current;
            const_iterator(std::unique_ptr<iterator_implementation> p);

          public:
            /// Allow comparison for equality
            bool operator==(const const_iterator &) const;
            /// Allow comparison for inequality
            bool operator!=(const const_iterator &) const;
            /// Allow the memory block to be accessed
            const_memory_block operator*() const;
            /// Move to the next chunk of data
            const const_iterator &operator++();
        };

        /// Fetch the start of the MIME data
        const_iterator begin() const;
        /// Fetch the end of the MIME data
        const_iterator end() const;

      protected:
        virtual std::unique_ptr<iterator_implementation> iterator() const = 0;

        mime(mime_headers headers, f5::u8view content_type);
    };


    /// A MIME implementation that can never have any body data.
    class FOST_INET_DECLSPEC empty_mime : public mime {
        struct empty_mime_iterator;
        std::unique_ptr<iterator_implementation> iterator() const;

      public:
        /// Construct an empty MIME body that cannot have data
        empty_mime(
                const mime_headers &headers = mime_headers(),
                const string &mime = "application/x-empty");

        /// Print the MIME out on the stream
        std::ostream &print_on(std::ostream &o) const;
        /// Check to see if the boundary is OK
        bool boundary_is_ok(const string &boundary) const;
    };

    /// A MIME envelope can contain a nested structure of other MIME containers
    class FOST_INET_DECLSPEC mime_envelope : public mime {
        struct mime_envelope_iterator;
        std::unique_ptr<iterator_implementation> iterator() const;
        string m_boundary;

      public:
        /// Construct the MIME envelope with optional headers
        mime_envelope(
                const mime_headers &headers = mime_headers(),
                const string &mime = "multipart/mixed");

        /// Print the MIME out on the stream
        std::ostream &print_on(std::ostream &o) const;
        /// Check that the boundary can be used
        bool boundary_is_ok(const string &boundary) const;

        /// The type of the attachments
        typedef std::list<boost::shared_ptr<mime>> items_type;
        /// The embedded MIME items
        accessors<items_type, fostlib::lvalue> items;

        /// Attach a MIME type
        template<typename M, typename P1>
        boost::shared_ptr<M> attach(const P1 &p1) {
            boost::shared_ptr<M> attachment(new M(p1));
            items().push_back(attachment);
            return attachment;
        }
        /// Attach a MIME type
        template<typename M, typename P1, typename P2>
        boost::shared_ptr<M> attach(const P1 &p1, const P2 &p2) {
            boost::shared_ptr<M> attachment(new M(p1, p2));
            items().push_back(attachment);
            return attachment;
        }
        /// Attach a MIME type
        template<typename M, typename P1, typename P2, typename P3>
        boost::shared_ptr<M> attach(const P1 &p1, const P2 &p2, const P3 &p3) {
            boost::shared_ptr<M> attachment(new M(p1, p2, p3));
            items().push_back(attachment);
            return attachment;
        }

        /// After all attachments have been added, use this to find a boundary
        const string &boundary();
    };

    /// A MIME container which always stores text
    class FOST_INET_DECLSPEC text_body : public mime {
        struct text_body_iterator;
        std::unique_ptr<iterator_implementation> iterator() const;

      public:
        text_body(
                f5::u8view text,
                mime_headers headers = mime_headers(),
                f5::u8view mime = "text/plain");

        /// Print the MIME out on the stream
        std::ostream &print_on(std::ostream &o) const;
        /// Check that the boundary can be used
        bool boundary_is_ok(const string &boundary) const;

        accessors<const utf8_string> text;

        f5::u8string body_as_string() const {
            return static_cast<f5::u8string>(text());
        }
    };

    /// A MIME container which represents binary data in memory
    class FOST_INET_DECLSPEC binary_body : public mime {
        struct binary_body_iterator;
        std::unique_ptr<iterator_implementation> iterator() const;

      public:
        /// The type of the data storage vector
        typedef std::vector<unsigned char> data_type;

        /// Construct an empty body
        binary_body(
                const mime_headers &headers = mime_headers(),
                const string &mime = "application/x-empty");
        /// Construct from a data block
        binary_body(
                const data_type &data,
                const mime_headers &headers = mime_headers(),
                const string &mime = "binary/octet-stream");
        /// Construct from a byte array
        binary_body(
                const char *begin,
                const char *end,
                const mime_headers &headers = mime_headers(),
                const string &mime = "binary/octet-stream");
        /// Construct from a signed byte array
        binary_body(
                const signed char *begin,
                const signed char *end,
                const mime_headers &headers = mime_headers(),
                const string &mime = "binary/octet-stream");
        /// Construct from a unsigned byte array
        binary_body(
                const unsigned char *begin,
                const unsigned char *end,
                const mime_headers &headers = mime_headers(),
                const string &mime = "binary/octet-stream");

        /// Display the data on a stream
        std::ostream &print_on(std::ostream &o) const;
        /// Check that the boundary can be used
        bool boundary_is_ok(const string &boundary) const;

        /// Allow direct read access to the data
        accessors<const data_type> data;
    };

    /// A MIME container which represents a file on disk
    class FOST_INET_DECLSPEC file_body : public mime {
        struct file_body_iteration;
        std::unique_ptr<iterator_implementation> iterator() const;

      public:
        file_body(
                const fostlib::fs::path &file,
                const mime_headers &headers = mime_headers(),
                const string &mime = "binary/octet-stream");

        /// Print the MIME out on the stream
        std::ostream &print_on(std::ostream &o) const;
        /// Check that the boundary can be used
        bool boundary_is_ok(const string &boundary) const;

        accessors<const fostlib::fs::path> filename;
    };


    /// Allow the UTF8 text of a MIME wrapper to be extracted.
    template<>
    struct FOST_INET_DECLSPEC coercer<utf8_string, mime> {
        /// Performs the coercion
        utf8_string coerce(mime const &m) {
            return utf8_string{m.body_as_string()};
        }
    };

    /// Allow the conversion of the MIME content to be extracted as a string
    template<>
    struct FOST_INET_DECLSPEC coercer<string, mime> {
        /// Performs the coercion
        string coerce(mime const &m) { return m.body_as_string(); }
    };

    /// Allow the conversion of MIME subclasses
    template<typename T, typename F>
    struct coercer<T, F, std::enable_if_t<std::is_base_of_v<mime, F>>> {
        T coerce(F const &f) {
            return fostlib::coerce<T>(reinterpret_cast<mime const &>(f));
        }
    };


    inline std::ostream &operator<<(std::ostream &o, const mime &m) {
        return m.print_on(o);
    }
#ifdef WIN32
    inline std::wostream &operator<<(std::wostream &o, const mime &m) {
        std::stringstream ss;
        ss << m;
        return o << coerce<string>(utf8_string(ss.str()));
    }
#endif


}


#endif // FOST_INTERNET_MIME_HPP
