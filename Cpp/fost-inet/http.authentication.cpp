#include "fost-inet.hpp"
#include <fost/datetime>
#include <fost/crypto>
#include <fost/exception/not_implemented.hpp>
#include <fost/http.authentication.fost.hpp>


using namespace fostlib;
using namespace std::literals;


namespace {


    void do_authn(
            const fostlib::string &api_key,
            const fostlib::string &secret,
            const std::set<fostlib::string> &headers_to_sign,
            fostlib::http::user_agent::request &request) {
        using namespace fostlib;

        fostlib::hmac signature(fostlib::sha1, secret);

        signature << request.method() << " " << request.address().pathspec()
                  << "\n";

        fostlib::string now = fostlib::coerce<fostlib::string>(
                std::chrono::system_clock::now());
        request.headers().set("X-FOST-Timestamp", now);
        signature << now << "\n";

        fostlib::string to_sign, signd = "X-FOST-Headers";
        for (std::set<fostlib::string>::const_iterator i(
                     headers_to_sign.begin());
             i != headers_to_sign.end(); ++i) {
            to_sign += "\n" + request.headers()[*i].value();
            signd += " " + *i;
        }
        signature << signd << to_sign << "\n";
        if (request.data().begin() != request.data().end())
            for (mime::const_iterator i(request.data().begin());
                 i != request.data().end(); ++i)
                signature << *i;
        else
            signature << fostlib::utf8_string(request.address()
                                                      .query()
                                                      .as_string()
                                                      .value_or("")
                                                      .underlying());

        request.headers().set("X-FOST-Headers", signd);
        request.headers().set(
                "Authorization",
                "FOST " + api_key + ":"
                        + fostlib::coerce<string>(
                                fostlib::coerce<fostlib::base64_string>(
                                        signature.digest())));
    }


}


void fostlib::http::fost_authentication(
        const fostlib::string &api_key,
        const fostlib::string &secret,
        const std::set<fostlib::string> &headers_to_sign,
        fostlib::http::user_agent::request &request) {
    do_authn(api_key, secret, headers_to_sign, request);
}

void fostlib::http::fost_authentication(
        fostlib::http::user_agent &ua,
        const fostlib::string &api_key,
        const fostlib::string &secret,
        const std::set<fostlib::string> &tosign) {
    ua.authentication(
            [api_key, secret, tosign](fostlib::http::user_agent::request &req) {
                do_authn(api_key, secret, tosign, req);
            });
}


fostlib::http::fost_authn::fost_authn(const string &m, bool a)
: error(m), authenticated(false), under_attack(a) {}
fostlib::http::fost_authn::fost_authn(
        std::shared_ptr<const mime::mime_headers> h)
: authenticated(true), under_attack(true), signed_headers(h) {}


fostlib::http::fost_authn fostlib::http::fost_authentication(
        std::function<nullable<string>(string)> key_mapping,
        server::request &request) {
    if (!request.data()->headers().exists("Authorization"))
        return fost_authn("No authorization header");

    std::pair<string, nullable<string>> authorization =
            partition(request.data()->headers()["Authorization"].value());

    if (authorization.first != "FOST" || not authorization.second) {
        return fost_authn("Non FOST authentication not implemented");
    } else {
        if (!request.data()->headers().exists("X-FOST-Headers"))
            return fost_authn("No signed headers found");

        split_type signed_headers =
                split(request.data()->headers()["X-FOST-Headers"].value(), " ");
        if (!signed_headers.size())
            return fost_authn("No signed headers found");

        if (std::find(
                    signed_headers.begin(), signed_headers.end(),
                    "X-FOST-Headers")
            == signed_headers.end())
            return fost_authn(
                    "X-FOST-Headers must be one of the signed headers");

        if (!request.data()->headers().exists("X-FOST-Timestamp"))
            return fost_authn("No X-FOST-Timestamp header found");

        auto const time_signed = coerce<std::chrono::system_clock::time_point>(
                json(request.data()->headers()["X-FOST-Timestamp"].value()));
        auto const now = std::chrono::system_clock::now();
        auto const oldest = now - 5 * 60s;
        auto const youngest = now + 5 * 60s;
        if (time_signed < oldest or time_signed > youngest) {
            return fost_authn("Clock skew is too high");
        }
        // boost::posix_time::ptime time_signed =
        //         coerce<boost::posix_time::ptime>(coerce<std::chrono::system_clock::time_point>(json(
        //                 request.data()->headers()["X-FOST-Timestamp"].value())));
        // boost::posix_time::ptime now =
        //         coerce<boost::posix_time::ptime>(std::chrono::system_clock::now());
        // boost::posix_time::ptime oldest(now - boost::posix_time::minutes(5));
        // boost::posix_time::ptime youngest(now +
        // boost::posix_time::minutes(5)); if (time_signed < oldest ||
        // time_signed > youngest)
        //     return fost_authn("Clock skew is too high");

        std::pair<string, nullable<string>> signature_partition =
                partition(authorization.second.value(), ":");
        if (not signature_partition.second) {
            return fost_authn("No FOST key:signature pair found");
        } else {
            const string &key = signature_partition.first;
            const string &signature = signature_partition.second.value();
            nullable<string> found_secret = key_mapping(key);
            if (not found_secret) {
                return fost_authn("Key not found", true);
            } else {
                const string &secret = found_secret.value();
                std::shared_ptr<mime::mime_headers> headers(
                        new mime::mime_headers);
                hmac document(&sha1, secret);
                document << request.method() << " " << request.file_spec()
                         << "\n"
                         << request.data()->headers()["X-FOST-Timestamp"].value()
                         << "\n";
                headers->add(
                        "X-FOST-Timestamp",
                        request.data()->headers()["X-FOST-Timestamp"]);
                for (split_type::const_iterator h(signed_headers.begin());
                     h != signed_headers.end(); ++h) {
                    std::stringstream ss;
                    ss << request.data()->headers()[*h];
                    document << string{ss.str()} << "\n";
                    headers->add(*h, request.data()->headers()[*h]);
                }
                if (coerce<string>(coerce<base64_string>(document.digest()))
                    == signature)
                    return fost_authn(headers);
                else
                    return fost_authn("Signature mismatch", true);
            }
        }
    }
}
fostlib::http::fost_authn fostlib::http::fost_authentication(
        const std::map<string, string> &keys,
        fostlib::http::server::request &request) {
    auto lookup = [&keys](const string &key) -> nullable<string> {
        const auto p = keys.find(key);
        if (p == keys.end())
            return null;
        else
            return p->second;
    };
    return fost_authentication(lookup, request);
}
