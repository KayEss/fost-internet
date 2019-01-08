/**
    Copyright 1999-2019, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include "fost-inet.hpp"
#include <fost/url.hpp>
#include <fost/string>

#include <fost/exception/not_null.hpp>


using namespace fostlib;


namespace {


    template<typename C>
    C digit(unsigned char dig) {
        if (dig < 0x0a) return dig + '0';
        if (dig < 0x10) return dig + 'A' - 0x0a;
        throw fostlib::exceptions::out_of_range<int>(
                L"Number to convert to hex digit is too big", 0, 0x10, dig);
    }
    template<typename S>
    S hex(unsigned char ch) {
        std::string num;
        num += '%';
        num += digit<typename std::string::value_type>((ch & 0xf0) >> 4);
        num += digit<typename std::string::value_type>(ch & 0x0f);
        return S{std::move(num)};
    }


    /**
     * Including the `=` sign here is a bit dodgy. It's OK to have it un-
     * encoded in the value part, but clearly if it's not encoded in key
     * then clearly things will go wrong. But if you put a `=` sign in
     * the query string key names you're pretty much asking for a
     * ton of trouble anyway, so....
     */
    const fostlib::utf8_string g_query_string_allowed(
            ".,:/\\_-*~="
            "0123456789"
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ");


}


/*
    fostlib::url::query_string
*/


fostlib::url::query_string::query_string() {}
fostlib::url::query_string::query_string(const ascii_printable_string &q)
: m_string(q) {}


void fostlib::url::query_string::append(
        const string &name, const nullable<string> &value) {
    m_string = null;
    m_query[name].push_back(value);
}


void fostlib::url::query_string::remove(const string &name) {
    const auto p(m_query.find(name));
    if (p != m_query.end()) {
        m_string = null;
        m_query.erase(p);
    }
}


std::size_t fostlib::url::query_string::has_key(const string &key) const {
    const auto p(m_query.find(key));
    if (p == m_query.end()) {
        return 0;
    } else {
        return p->second.size();
    }
}


nullable<string> fostlib::url::query_string::operator[](const string &k) const {
    const auto p(m_query.find(k));
    if (p == m_query.end() || p->second.size() == 0u) {
        return null;
    } else {
        return *p->second.begin();
    }
}


namespace {
    const std::vector<nullable<string>> c_empty_list;
}
const std::vector<nullable<string>> &
        fostlib::url::query_string::at(const string &key) const {
    const auto p(m_query.find(key));
    if (p == m_query.end()) {
        return c_empty_list;
    } else {
        return p->second;
    }
}


namespace {
    ascii_printable_string query_string_encode(const string &s) {
        std::string r;
        auto const i(coerce<utf8_string>(s));
        for (auto const c : i.memory()) {
            if (g_query_string_allowed.underlying().find(c) == std::string::npos) {
                r += hex<std::string>(c);
            } else {
                r += c;
            }
        }
        return ascii_printable_string{std::move(r)};
    }
    nullable<ascii_printable_string>
            query_string_encode(const nullable<string> &s) {
        if (not s) {
            return null;
        } else {
            return query_string_encode(s.value());
        }
    }
}
const nullable<ascii_printable_string> &
        fostlib::url::query_string::as_string() const {
    if (not m_string) {
        nullable<ascii_printable_string> r;
        for (auto it(m_query.begin()); it != m_query.end(); ++it) {
            for (auto v(it->second.begin()); v != it->second.end(); ++v) {
                r = concat(r, ascii_printable_string("&"),
                               concat(query_string_encode(it->first)
                                              + ascii_printable_string("="),
                                      query_string_encode(*v)));
            }
        }
        m_string = r;
    }
    return m_string;
}
