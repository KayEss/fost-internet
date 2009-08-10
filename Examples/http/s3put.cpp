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
    // Check we have the minimum number of command line arguments
    if ( args.size() < 3 ) {
        o << L"Must supply at least one file and a S3 location to put it to" << std::endl;
        return 1;
    }
    // Create the S3 account object
    boost::scoped_ptr< aws::s3 > s3account;
    if ( !args.commandSwitch("a").isnull() )
        s3account.reset(new aws::s3(args.commandSwitch("a").value()));
    else
        s3account.reset(new aws::s3);
    // Work out the base URL and show the user what is about to happen
    url base(args[2].value());
    o << L"Base URL: " << base << std::endl;
    return 0;
}
