/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/datetime>
#include <fost/crypto>

#include <fost/detail/http.authentication.fost.hpp>

#include <boost/lambda/lambda.hpp>


void fostlib::http::fost_authentication(
    const fostlib::string &api_key, const fostlib::string &secret,
    const std::set< fostlib::string > &headers_to_sign,
    fostlib::http::user_agent::request &request
) {
    fostlib::hmac signature(fostlib::sha1, secret);

    signature << request.method() << " " << request.address().pathspec().underlying().underlying() << "\n";

    fostlib::string now = fostlib::coerce< fostlib::string >( fostlib::timestamp::now() );
    request.headers().add( L"X-FOST-Timestamp", now );
    signature << now << "\n";

    fostlib::string to_sign, signd = L"X-FOST-Headers";
    for ( std::set< fostlib::string >::const_iterator i( headers_to_sign.begin() ); i != headers_to_sign.end(); ++i ) {
        to_sign += L"\n" + request.headers()[ *i ].value();
        signd += L" " + *i;
    }
    signature << signd << to_sign << "\n";
    for ( mime::const_iterator i(request.data().begin()); i != request.data().end(); ++i )
        signature << *i;

    request.headers().add( L"X-FOST-Headers", signd );
    request.headers().add( L"Authorization", L"FOST " + api_key + L":" +
        fostlib::coerce< string >( fostlib::coerce< fostlib::base64_string >(signature.digest()) )
    );
}

