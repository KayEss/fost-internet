/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"
#include <fost/detail/http.server.hpp>


using namespace fostlib;


FSL_TEST_SUITE( http_server );


FSL_TEST_FUNCTION( mock ) {
    http::server::request req("GET", url::filepath_string("/"), std::auto_ptr< mime >( new empty_mime ));
    empty_mime response;
    FSL_CHECK_EXCEPTION(req( response ), exceptions::null&);
}
