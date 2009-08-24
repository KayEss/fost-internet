/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/aws>
#include <fost/test>


using namespace fostlib;


namespace {
    const setting< string > c_account("fost-internet/Cpp/fost-aws-test/test-s3authen.cpp",
        "Amazon S3", "Default account name", "test", false
    );
    const setting< string > c_api_key("fost-internet/Cpp/fost-aws-test/test-s3authen.cpp",
        "S3 account/test", "API key", "0PN5J17HBGZHT7JJ3X82", true
    );
    const setting< string > c_api_secret("fost-internet/Cpp/fost-aws-test/test-s3authen.cpp",
        "S3 account/test", "API secret", "uV3F3YluFJax1cknvbcGwgjvx4QpvB+leU8dUj2o", true
    );
}


FSL_TEST_SUITE( s3_authentication );


FSL_TEST_FUNCTION( get ) {
    aws::s3::bucket bucket(ascii_string("johnsmith"));
    http::user_agent::request request("GET", url("http://johnsmith.s3.amazonaws.com/photos/puppy.jpg"));
    request.headers().set("Date", L"Tue, 27 Mar 2007 19:36:42 +0000");
    request.headers().set("Content-Type", L"");
    aws::s3::rest_authentication("test", bucket.name(), request);
    FSL_CHECK_EQ(request.headers()["Authorization"].value(),
        L"AWS 0PN5J17HBGZHT7JJ3X82:xXjDGYUmKxnwqr5KXNPGldn5LbA="
    );
}

