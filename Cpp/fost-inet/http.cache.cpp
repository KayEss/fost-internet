/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet.hpp"
#include <fost/ua/cache.detail.hpp>

#include <fost/base32>
#include <fost/crypto>


fostlib::setting<std::optional<fostlib::string>> const
        fostlib::ua::c_cache_folder{
                __FILE__, "HTTP cache", "Cache folder", {}, true};

fostlib::setting<bool> const fostlib::ua::c_force_no_http_requests{
        __FILE__, "HTTP cache", "Force no network requests", false, true};


fostlib::json fostlib::ua::get_json(url const &, headers const &) {
    return json{};
}


void fostlib::ua::expect_get(url const &, json, headers const &) {}


fostlib::string fostlib::ua::cache_key(
        f5::u8view method, url const &u, headers const &) {
    fostlib::digester hash{fostlib::sha256};
    if (u.fragment()) {
        auto uf = u;
        uf.fragment({});
        hash << method << " " << uf.as_string() << "\n";
    } else {
        hash << method << " " << u.as_string() << "\n";
    }
    return f5::u8view{fostlib::coerce<fostlib::base32c_string>(
                              array_view<const unsigned char>{hash.digest()})}
            .substr_pos(0, 52);
}
