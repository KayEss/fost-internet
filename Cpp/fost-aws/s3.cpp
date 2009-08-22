/*
    Copyright 2008, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-aws.hpp"
#include <fost/detail/s3.hpp>


using namespace fostlib;
using namespace fostlib::aws::s3;


/*
    fostlib::aws::s3::bucket
*/


namespace {

}


fostlib::aws::s3::bucket::bucket( const ascii_string &name )
: m_ua(fostlib::url("https://aws.amazon.com/")), name( name ) {
}


file_info fostlib::aws::s3::bucket::stat(const boost::filesystem::wpath &location) const {
    return file_info(m_ua, name(), location);
}


void fostlib::aws::s3::bucket::put(const boost::filesystem::wpath &file, const boost::filesystem::wpath &location) const {
    http::user_agent::request request("PUT", url(
        m_ua.base(), boost::filesystem::wpath(coerce< std::wstring >(name()) / location)
    ), file );
    std::auto_ptr< http::user_agent::response > response(m_ua(request));
    throw fostlib::exceptions::not_implemented("fostlib::aws::s3::bucket::put(const boost::filesystem::wpath &file, const boost::filesystem::wpath &location) const -- with response status", fostlib::coerce< fostlib::string >( response->status() ));
}


/*
    fostlib::aws::s3::file_info
*/


fostlib::aws::s3::file_info::file_info(const http::user_agent &ua, const ascii_string &bucket, const boost::filesystem::wpath &location )
: path( location ) {
    http::user_agent::request request("HEAD", url(
        ua.base(), boost::filesystem::wpath(coerce< std::wstring >(bucket)) / location
    ));
    std::auto_ptr< http::user_agent::response > response(ua(request));

    switch ( response->status() ) {
    case 404:
        break;
    default:
        throw fostlib::exceptions::not_implemented("fostlib::aws::s3::file_info::file_info( const fostlib::aws::s3::bucket &bucket, const boost::filesystem::wpath &location ) -- with status code", fostlib::coerce< fostlib::string >( response->status() ));
    }
}


bool fostlib::aws::s3::file_info::exists() const {
    return false;
}

