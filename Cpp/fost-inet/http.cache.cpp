/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet.hpp"
#include <fost/ua/cache.detail.hpp>
#include <fost/ua/exceptions.hpp>
#include <fost/http.useragent.hpp>

#include <fost/base32>
#include <fost/crypto>
#include <fost/insert>
#include <f5/threading/map.hpp>


fostlib::module const fostlib::ua::c_fost_inet_ua{c_fost_inet, "ua"};


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


    fostlib::json make_request(
            f5::u8view const method,
            fostlib::url const &url,
            fostlib::json body,
            fostlib::ua::headers headers) {
        headers.add("Accept", "application/json");
        fostlib::http::user_agent ua;
        fostlib::http::user_agent::request req{
                method, url, fostlib::json::unparse(body, false),
                std::move(headers)};
        auto response = ua(req);
        if (response->status() == 404 || response->status() == 410) {
            throw fostlib::ua::resource_not_found{url};
        }
        return response->body()->body_as_json();
    }


    fostlib::json check_cache(
            f5::u8view const method,
            fostlib::url const &url,
            fostlib::json body,
            fostlib::ua::headers headers) {
        return make_request(method, url, std::move(body), std::move(headers));
    }


}


fostlib::json fostlib::ua::request_json(
        f5::u8view const method, url const &url, json body, headers headers) {
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
        } else if (not c_cache_folder.value() && c_force_no_http_requests.value()) {
            throw no_expectation{"Expectations run out", method, url,
                                 std::move(body), std::move(headers)};
        } else {
            ret = check_cache(method, url, std::move(body), std::move(headers));
        }
    });
    if (expectation_found) {
        return ret;
    } else if (not c_cache_folder.value() && c_force_no_http_requests.value()) {
        throw no_expectation{"Expectation was never set", method, url,
                             std::move(body), std::move(headers)};
    } else {
        return check_cache(method, url, std::move(body), std::move(headers));
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


/// ## Exceptions


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


fostlib::ua::resource_not_found::resource_not_found(url const &u)
: exception{f5::u8view{u.as_string()}} {}
const wchar_t *const fostlib::ua::resource_not_found::message() const {
    return L"Resource not found";
}
