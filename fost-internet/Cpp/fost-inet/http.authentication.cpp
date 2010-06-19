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
