/**
    Copyright 1999-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet.hpp"
#include <fost/mime.hpp>
#include <fost/exception/parse_error.hpp>
#include <fost/insert>
#include <fost/push_back>
#include <fost/url.hpp>


using namespace fostlib;


void fostlib::parse_cookies(fostlib::headers_base::content &cookie) {
    const auto unescape = [](const auto &s) {
        const fostlib::url::filepath_string fs{s};
        return fostlib::coerce<fostlib::string>(fs);
    };
    auto cookies = fostlib::split(cookie.value(), "; ");
    for (const auto &cv : cookies) {
        const auto parts = partition(cv, "=");
        cookie.subvalue(
                unescape(parts.first),
                unescape(parts.second.value_or(fostlib::string())));
    }
    cookie.value(fostlib::string());
}


/**
    ## fostlib::headers_base
*/


fostlib::headers_base::headers_base() : fold_limit(78) {}
fostlib::headers_base::~headers_base() {}


namespace {
    typedef std::pair<string, fostlib::nullable<string>> string_pair;
    const fostlib::string c_mime_newline("\r\n");

    bool is_whitespace(wchar_t c) { return c == L' ' || c == L'\t'; }

    string_pair precise_partition(
            const fostlib::nullable<string> &s,
            const fostlib::string &separator) {
        if (not s) return string_pair();
        size_t cut_position = s.value().find(separator);

        if (cut_position == fostlib::string::npos)
            return string_pair(s.value(), fostlib::nullable<string>());
        fostlib::string second =
                s.value().substr(cut_position + separator.code_points());
        return string_pair(
                s.value().substr(0, cut_position),
                second.empty() ? fostlib::nullable<string>() : second);
    }

    string_pair mime_partition(const fostlib::nullable<string> &headers) {
        if (not headers) return string_pair();
        string_pair part = precise_partition(headers, c_mime_newline);
        while (part.second && is_whitespace(part.second.value().at(0))) {
            string_pair new_part =
                    precise_partition(part.second.value(), c_mime_newline);
            part.first += new_part.first;
            part.second = new_part.second;
        }
        return part;
    }
}

void fostlib::headers_base::parse(const string &headers) {
    /// This implementation ignores character encodings
    /// - assumes UTF-8 which won't work for mail headers
    for (string_pair lines(mime_partition(headers)); !lines.first.empty();
         lines = mime_partition(lines.second)) {
        const string_pair line(partition(lines.first, ":"));
        const std::pair<string, content> parsed(
                value(line.first, line.second.value_or(fostlib::string())));
        if (parsed.first.find('_') == std::string::npos) {
            add(parsed.first, parsed.second);
        }
    }
}

std::pair<fostlib::string, headers_base::content> fostlib::headers_base::value(
        const fostlib::string &name, const fostlib::string &value) {
    return std::make_pair(name, content(value));
}

bool fostlib::headers_base::exists(const fostlib::string &n) const {
    return m_headers.find(n) != m_headers.end();
}
namespace {
    // Done here so it is initialised on DLL loading.
    // Should g'tee no multithreading problem as this isn't going to be used
    // in the DLL initialisation
    const headers_base::content g_stat;
}
const headers_base::content &
        fostlib::headers_base::operator[](const fostlib::string &n) const {
    header_store_type::const_iterator p(m_headers.find(n));
    if (p == m_headers.end())
        return g_stat;
    else
        return (*p).second;
}

void fostlib::headers_base::set(f5::u8view n, content v) {
    m_headers.erase(m_headers.lower_bound(n), m_headers.upper_bound(n));
    add(n, std::move(v));
}
void fostlib::headers_base::add(f5::u8view n, content v) {
    m_headers.insert(m_headers.upper_bound(n), std::make_pair(n, std::move(v)));
}
void fostlib::headers_base::set_subvalue(
        const string &n, const string &k, const string &v) {
    header_store_type::iterator lower(m_headers.lower_bound(n));
    if (lower == m_headers.end()) {
        set(n);
        lower = m_headers.lower_bound(n);
    }
    header_store_type::iterator upper(m_headers.upper_bound(n));
    for (header_store_type::iterator h(lower); h != upper; ++h)
        (*h).second.subvalue(k, v);
}

fostlib::headers_base::const_iterator fostlib::headers_base::begin() const {
    return m_headers.begin();
}
fostlib::headers_base::const_iterator fostlib::headers_base::end() const {
    return m_headers.end();
}

/// NOTE: probably better if implemented as an ostream-derived class.
namespace {
    std::string fold(const std::string &s, size_t line_limit) {
        std::stringstream ss;
        std::string left_string = s;

        int exceed_len;
        while ((exceed_len = left_string.length() - line_limit) > 0) {
            size_t cut_point = left_string.find_last_of(
                    " \t", left_string.length() - exceed_len);
            if (cut_point == std::string::npos) break;
            ss << left_string.substr(0, cut_point) << "\r\n";
            left_string = left_string.substr(cut_point);
        }
        ss << left_string;
        return ss.str();
    }
}

std::ostream &fostlib::operator<<(
        std::ostream &o, const fostlib::headers_base &headers) {
    for (headers_base::const_iterator i(headers.begin()); i != headers.end();
         ++i) {
        std::stringstream ss;
        ss << coerce<ascii_string>(i->first).underlying() << ": " << i->second
           << "\r\n";
        if (not headers.fold_limit()) {
            o << ss.str();
        } else {
            o << fold(ss.str(), headers.fold_limit().value());
        }
    }
    return o;
}


json fostlib::detail::from_headers(const headers_base &h) {
    json values = json::object_t();
    for (headers_base::const_iterator i(h.begin()); i != h.end(); ++i) {
        if (values.has_key(i->first)) {
            if (values[i->first].isarray()) {
                push_back(values, i->first, coerce<json>(i->second));
            } else {
                json arr = json::array_t();
                push_back(arr, values[i->first]);
                push_back(arr, fostlib::coerce<json>(i->second));
                jcursor(i->first).replace(values, arr);
            }
        } else {
            insert(values, i->first, coerce<json>(i->second));
        }
    }
    return values;
}


/*
    fostlib::headers_base::content
*/


fostlib::headers_base::content::content() {}
fostlib::headers_base::content::content(f5::u8view val) : value(val) {}
fostlib::headers_base::content::content(
        f5::u8view val, const std::map<string, string> &args)
: m_subvalues(args.begin(), args.end()), value(val) {}
fostlib::headers_base::content::content(const json &values, const string &root) {
    try {
        if (values.isobject()) {
            for (json::const_iterator item(values.begin());
                 item != values.end(); ++item) {
                const auto key = coerce<string>(item.key());
                const auto itemstr = (*item).isarray()
                        ? fostlib::json::unparse(*item, false)
                        : coerce<string>(*item);
                if (key == root) {
                    value(itemstr);
                } else {
                    m_subvalues[key] = itemstr;
                }
            }
        } else if (values.isatom()) {
            value(coerce<string>(values));
        } else {
            throw exceptions::not_implemented(
                    __func__, "Can't set header values from this JSON", values);
        }
    } catch (exceptions::exception &e) {
        insert(e.data(), "headers_base", "content", "values", values);
        insert(e.data(), "headers_base", "content", "root", root);
        throw;
    }
}


headers_base::content &fostlib::headers_base::content::subvalue(
        const string &k, const string &v) {
    m_subvalues[k] = v;
    return *this;
}

nullable<string>
        fostlib::headers_base::content::subvalue(const string &k) const {
    std::map<string, string, detail::ascii_iless>::const_iterator p(
            m_subvalues.find(k));
    if (p == m_subvalues.end())
        return null;
    else
        return (*p).second;
}

headers_base::content::const_iterator
        fostlib::headers_base::content::begin() const {
    return m_subvalues.begin();
}
headers_base::content::const_iterator
        fostlib::headers_base::content::end() const {
    return m_subvalues.end();
}

std::ostream &
        fostlib::operator<<(std::ostream &o, const headers_base::content &v) {
    /// Without the `static_cast` here the Android NDK compiler goes super weird
    o << static_cast<std::string_view>(v.value());
    for (auto const &i : v) {
        o << "; " << i.first << "=\"" << i.second << "\"";
    }
    return o;
}
