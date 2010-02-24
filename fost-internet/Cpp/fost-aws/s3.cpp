/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-aws.hpp"
#include <fost/detail/s3.hpp>

#include <boost/lambda/bind.hpp>


using namespace fostlib;
using namespace fostlib::aws::s3;


/*
    fostlib::aws::s3::bucket
*/

const setting< string > fostlib::aws::s3::bucket::s_account_name(
    "fost-internet/Cpp/fost-aws/s3.cpp",
    "Amazon S3", "Default account name", "default", true
);

namespace {
    std::auto_ptr< http::user_agent::response > s3do(const http::user_agent &ua, http::user_agent::request &request) {
        std::auto_ptr< http::user_agent::response > response = ua(request);
        if ( response->status() == 403 ) {
            exceptions::not_implemented exception("S3 request resulting in 403 forbidden");
            exception.info() << response->body() << std::endl;
            throw exception;
        }
        return response;
    }
}


fostlib::aws::s3::bucket::bucket( const ascii_string &name )
: m_ua(url(coerce< string >( ascii_string("https://") + name + ascii_string(".s3.amazonaws.com/") ))), name( name ) {
    m_ua.authentication(boost::function< void ( http::user_agent::request & ) >(boost::lambda::bind(
        rest_authentication, s_account_name.value(), name, boost::lambda::_1
    )));
}


file_info fostlib::aws::s3::bucket::stat(const boost::filesystem::wpath &location) const {
    return file_info(m_ua, name(), location);
}


void fostlib::aws::s3::bucket::put(const boost::filesystem::wpath &file, const boost::filesystem::wpath &location) const {
    http::user_agent::request request("PUT", url(m_ua.base(), location), file);
    std::auto_ptr< http::user_agent::response > response(s3do(m_ua, request));
    switch ( response->status() ) {
        case 200:
            break;
        default:
            exceptions::not_implemented exception(L"fostlib::aws::s3::bucket::put(const boost::filesystem::wpath &file, const boost::filesystem::wpath &location) const -- with response status " + fostlib::coerce< fostlib::string >( response->status() ));
            exception.info() << response->body() << std::endl;
            throw exception;
    }
}


/*
    fostlib::aws::s3::file_info
*/


namespace {
    std::auto_ptr< http::user_agent::response > init_file_info(const http::user_agent &ua, const url &u) {
        http::user_agent::request r("HEAD", u);
        return s3do(ua, r);
    }
}
fostlib::aws::s3::file_info::file_info(const http::user_agent &ua, const ascii_string &bucket, const boost::filesystem::wpath &location )
: m_response( init_file_info(ua, url(ua.base(), location)).release() ), path( location ) {
    switch ( m_response->status() ) {
        case 200:
        case 404:
            break;
        default:
            throw fostlib::exceptions::not_implemented("fostlib::aws::s3::file_info::file_info( const fostlib::aws::s3::bucket &bucket, const boost::filesystem::wpath &location ) -- with status code", fostlib::coerce< fostlib::string >( m_response->status() ));
    }
}


bool fostlib::aws::s3::file_info::exists() const {
    return m_response->status() == 200;
}
nullable< string > fostlib::aws::s3::file_info::md5() const {
    if ( exists() && m_response->body().headers().exists(L"ETag") )
        return m_response->body().headers()[L"ETag"].value();
    else
        return null;
}
