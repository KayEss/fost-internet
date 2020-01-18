/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet.hpp"
#include <fost/ua/cache.detail.hpp>

#include <fost/base32>
#include <fost/crypto>
#include <fost/insert>
#include <f5/threading/map.hpp>


fostlib::setting<std::optional<fostlib::string>> const
        fostlib::ua::c_cache_folder{
                __FILE__, "HTTP cache", "Cache folder", {}, true};

fostlib::setting<bool> const fostlib::ua::c_force_no_http_requests{
        __FILE__, "HTTP cache", "Force no network requests", false, true};


namespace {
    struct expect {
        expect(fostlib::json j) : used{false}, items{std::move(j)} {}

        /// The `used` flag is needed to track when an idempotent
        /// expectation has been re-used, so we know that the
        /// expectation should be replaced with a new value
        bool used;
        std::vector<fostlib::json> items;
    };
    f5::tsmap<f5::u8string, expect> g_expectations;
}


fostlib::json fostlib::ua::request_json(
        f5::u8view const method,
        url const &url,
        fostlib::json body,
        headers const &headers) {
    fostlib::json ret;
    auto const key = cache_key(method, url, headers);
    auto const expectation_found = g_expectations.alter(key, [&](::expect &e) {
        if (not e.items.empty()) {
            ret = e.items.front();
            if (e.items.size() > 1 || not idempotent(method)) {
                e.items.erase(e.items.begin());
            } else {
                e.used = true;
            }
        } else {
            throw no_expectation{"Expectations run out", method, url,
                                 std::move(body), headers};
        }
    });
    if (expectation_found) {
        return ret;
    } else {
        throw no_expectation{"Expectation was never set", method, url,
                             std::move(body), headers};
    }
}


fostlib::ua::ua_test::ua_test() { clear_expectations(); }


void fostlib::ua::clear_expectations() { g_expectations.clear(); }


void fostlib::ua::expect(
        f5::u8view const method,
        url const &url,
        json r,
        headers const &headers) {
    auto key = cache_key(method, url, headers);
    g_expectations.add_if_not_found(
            key, [r]() { return ::expect{r}; },
            [r](::expect &e) {
                if (e.used) {
                    e.used = false;
                    e.items[0] = r;
                } else {
                    e.items.push_back(r);
                }
            });
}


f5::u8string fostlib::ua::cache_key(
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


bool fostlib::ua::idempotent(f5::u8view const method) {
    return method == "GET";
}


fostlib::ua::no_expectation::no_expectation(
        f5::u8view message,
        f5::u8view method,
        url const &url,
        fostlib::json body,
        headers const &headers)
: exception(message) {
    insert(data(), "method", method);
    insert(data(), "url", url);
    insert(data(), "body", body);
    insert(data(), "headers", headers);
}


const wchar_t *const fostlib::ua::no_expectation::message() const {
    return L"No expectation found for request";
}
