/**
    Copyright 1999-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#ifndef FOST_HEADERS_HPP
#define FOST_HEADERS_HPP
#pragma once


#include <fost/core>
#include <fost/pointers>


namespace fostlib {


    namespace detail {


        /// Case insensitive comparison function for ASCII letters
        inline bool ascii_iless_compare(utf32 c1, utf32 c2) {
            if (c1 < 0x7f && c2 < 0x7f) {
                const std::locale &loc(std::locale::classic());
                return std::tolower(c1, loc) < std::tolower(c2, loc);
            } else {
                return c1 < c2;
            }
        }


        /// Case insensitive version of std::less for chars in the ASCII range
        struct ascii_iless : std::binary_function<string, string, bool> {
            bool operator()(const string &c1, const string &c2) const {
                return std::lexicographical_compare(
                        c1.begin(), c1.end(), c2.begin(), c2.end(),
                        ascii_iless_compare);
            }
        };


    }


    /// An abstract base class used to describe headers as they appear in
    /// protocols like SMTP and HTTP.
    class FOST_INET_DECLSPEC FSL_ABSTRACT headers_base {
      public:
        class content;

      private:
        typedef std::multimap<string, content, detail::ascii_iless>
                header_store_type;

      public:
        /// Construct empty headers
        headers_base();
        /// Allow the headers to be sub-classed
        virtual ~headers_base();

        /// Parse the string and add the found headers
        void parse(const string &headers);

        /// Returns true if a specified header exists
        bool exists(const string &) const;
        /// Allows a header to be set, but without any value
        void set(f5::u8view name) { set(name, content{}); }
        /// Allows a header to be given a specified value
        void set(f5::u8view name, content);
        void set(f5::u8view name, f5::u8view value) {
            set(name, content{value});
        }
        template<std::size_t N, std::size_t V> // Delete when the below is deleted
        void set(char const (&n)[N], char const (&v)[V]) {
            set(f5::u8view{n}, content{f5::u8view{v}});
        }
        /// Allows a header to be given a set of values
        void set(f5::u8view name, const json &j, f5::u8view r = {}) {
            set(name, content(j, r));
        }
        /// Adds a header with a given name and content
        void add(f5::u8view name, content);
        void add(f5::u8view name, f5::u8view c) { add(name, content{c}); }
        /// Allow a specified sub-value on the specified header to be set
        void set_subvalue(const string &name, const string &k, const string &v);
        /// Fetches a header
        const content &operator[](const string &) const;

        /// Allow the fields to be iterated
        typedef header_store_type::const_iterator const_iterator;
        /// The beginning of the header fields
        const_iterator begin() const;
        /// The end of the header fields
        const_iterator end() const;

        /// The character limit before folding, defaults to 78
        accessors<nullable<std::size_t>> fold_limit;

        /// The content of header fields
        class FOST_INET_DECLSPEC content {
            std::map<string, string, detail::ascii_iless> m_subvalues;

          public:
            /// Create empty content for a header value
            content();
            /// Create header value content from a string
            content(f5::u8view);
            /// Create header value content from a string with sub-values
            content(f5::u8view, const std::map<string, string> &);
            /// Create header value content from JSON specifying the root key
            explicit content(const json &values, const string &root = string());

            /// The main value of the header field
            accessors<string> value;

            /// Set a field's sub-value
            content &subvalue(const string &k, const string &v);
            /// Access a field's sub-value
            nullable<string> subvalue(const string &k) const;

            /// Allows the sub-values to be iterated
            typedef std::map<string, string, detail::ascii_iless>::const_iterator
                    const_iterator;
            /// The start of the sub-values
            const_iterator begin() const;
            /// The end of the sub-values
            const_iterator end() const;
        };

      protected:
        virtual std::pair<string, content>
                value(const string &name, const string &value) = 0;

      private:
        header_store_type m_headers;
    };


    /// Parse the headers and put their values into the supplied
    /// content header.
    void parse_cookies(fostlib::headers_base::content &);


    /// Allow headers to be written to a narrow stream
    FOST_INET_DECLSPEC std::ostream &
            operator<<(std::ostream &o, const headers_base &headers);
    /// Allow header field values to be written to a narrow stream
    FOST_INET_DECLSPEC std::ostream &
            operator<<(std::ostream &o, const headers_base::content &value);

    /// Allow header content value to be turned to a string
    template<>
    struct coercer<string, headers_base::content> {
        string coerce(const headers_base::content &c) const {
            std::stringstream ss;
            ss << c;
            return string(ss.str().c_str());
        }
    };
    /// Allow header content value to be turned to JSON
    template<>
    struct coercer<json, headers_base::content> {
        json coerce(const headers_base::content &c) const {
            return json(fostlib::coerce<string>(c));
        }
    };

    namespace detail {
        FOST_INET_DECLSPEC json from_headers(const headers_base &);
    }
    /// Allow a full set of headers to be converted to JSON
    template<typename T>
    struct coercer<json, T, std::enable_if_t<std::is_base_of_v<headers_base, T>>> {
        json coerce(const headers_base &h) const {
            return detail::from_headers(h);
        }
    };


}


#endif // FOST_HEADERS_HPP
