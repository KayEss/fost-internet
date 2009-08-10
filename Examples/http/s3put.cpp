/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/cli>
#include <fost/main.hpp>
#include <fost/aws>


using namespace fostlib;


FSL_MAIN(
    L"fget",
    L"Amazon S3 client -- push to S3\nCopyright (C) 2008-2009, Felspar Co. Ltd."
)( fostlib::ostream &o, fostlib::arguments &args ) {
    if ( args.size() < 3 ) {
        o << L"Must supply at least one file and a S3 location to put it to" << std::endl;
        return 1;
    }
    url base(args[2].value());
    o << L"Base URL: " << base << std::endl;
    return 0;
}
