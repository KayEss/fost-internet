/*
    Copyright 2008-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/datetime>
#include <fost/crypto>

#include <fost/http.authentication.fost.hpp>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>


using namespace fostlib;


namespace {


    void do_authn(
        const fostlib::string &api_key, const fostlib::string &secret,
        const std::set< fostlib::string > &headers_to_sign,
        fostlib::http::user_agent::request &request
    ) {
        using namespace fostlib;

        fostlib::hmac signature(fostlib::sha1, secret);

        signature << request.method() << " "
            << request.address().pathspec().underlying().underlying().c_str()
            << "\n";

        fostlib::string now =
            fostlib::coerce< fostlib::string >( fostlib::timestamp::now() );
        request.headers().set( L"X-FOST-Timestamp", now );
        signature << now << "\n";

        fostlib::string to_sign, signd = L"X-FOST-Headers";
        for (
            std::set< fostlib::string >::const_iterator i( headers_to_sign.begin() );
            i != headers_to_sign.end(); ++i
        ) {
            to_sign += L"\n" + request.headers()[ *i ].value();
            signd += L" " + *i;
        }
        signature << signd << to_sign << "\n";
        if ( request.data().begin() != request.data().end() )
            for (
                mime::const_iterator i(request.data().begin());
                i != request.data().end(); ++i
            )
                signature << *i;
        else
            signature << fostlib::utf8_string(
                request.address().query().as_string().value("").underlying()
            );

        request.headers().set( L"X-FOST-Headers", signd );
        request.headers().set( L"Authorization", L"FOST " + api_key + L":" +
            fostlib::coerce< string >(
                fostlib::coerce< fostlib::base64_string >(signature.digest())
            )
        );
    }


}


void fostlib::http::fost_authentication(
    const fostlib::string &api_key, const fostlib::string &secret,
    const std::set< fostlib::string > &headers_to_sign,
    fostlib::http::user_agent::request &request
) {
    do_authn(api_key, secret, headers_to_sign, request);
}

void fostlib::http::fost_authentication(
    fostlib::http::user_agent &ua,
    const fostlib::string &api_key, const fostlib::string &secret,
    const std::set< fostlib::string > &tosign
) {
    ua.authentication(
        boost::function<
            void ( fostlib::http::user_agent::request& )
        >(boost::lambda::bind(
            do_authn, api_key, secret, tosign, boost::lambda::_1
        ))
    );
}


fostlib::http::fost_authn::fost_authn(const string &m, bool a)
: error(m), authenticated(false), under_attack(a) {
}


fostlib::http::fost_authn fostlib::http::fost_authentication(
    boost::function< nullable< string > ( string ) > key_mapping,
    server::request &request
) {
    if ( !request.data()->headers().exists("Authorization") )
        return fost_authn("No authorization header");

    std::pair< string, nullable<string> > authorization =
        partition(request.data()->headers()["Authorization"].value());

    if ( authorization.first != "FOST" || authorization.second.isnull() )
        return fost_authn("Non FOST authentication not implemented");
    else {
        if ( !request.data()->headers().exists("X-FOST-Headers") )
            return fost_authn("No signed headers found");

        split_type signed_headers = split(
            request.data()->headers()["X-FOST-Headers"].value(), " ");
        if ( !signed_headers.size() )
            return fost_authn("No signed headers found");

        if ( std::find(signed_headers.begin(), signed_headers.end(),
                "X-FOST-Headers") == signed_headers.end() )
            return fost_authn("X-FOST-Headers must be one of the signed headers");

        if ( !request.data()->headers().exists("X-FOST-Timestamp") )
            return fost_authn("No X-FOST-Timestamp header found");

        // TODO Switch to using a proper parser here so errors can be caught
        boost::posix_time::ptime time_signed =
            coerce<boost::posix_time::ptime>(coerce<timestamp>(
                json(request.data()->headers()["X-FOST-Timestamp"].value())));
        boost::posix_time::ptime now =
            coerce<boost::posix_time::ptime>(timestamp::now());
        boost::posix_time::ptime oldest(now - boost::posix_time::minutes(5));
        boost::posix_time::ptime youngest(now + boost::posix_time::minutes(5));
        if ( time_signed < oldest || time_signed > youngest )
            return fost_authn("Clock skew is too high");

        std::pair< string, nullable<string> > signature_partition =
            partition(authorization.second.value(), ":");
        if ( signature_partition.second.isnull() )
            return fost_authn("No FOST key:signature pair found");
        else {
            const string &key = signature_partition.first;
            const string &signature = signature_partition.second.value();
            nullable<string> found_secret = key_mapping(key);
            if ( found_secret.isnull() )
                return fost_authn("Key not found", true);
            else {
                const string &secret = found_secret.value();
                return fost_authn("Not implemented", true);
            }
        }
    }
}
namespace {
    nullable<string> lookup(
        const std::map<string, string> &keys, const string &key
    ) {
        std::map<string, string>::const_iterator p = keys.find(key);
        if ( p == keys.end() )
            return null;
        else
            return p->second;
    }
}
fostlib::http::fost_authn fostlib::http::fost_authentication(
    const std::map< string, string > &keys, fostlib::http::server::request &request
) {
    return fost_authentication(boost::lambda::bind(lookup, keys, boost::lambda::_1), request);
}
