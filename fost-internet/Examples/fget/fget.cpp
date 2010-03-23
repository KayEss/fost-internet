/*
    Copyright 2008-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/cli>
#include <fost/main.hpp>
#include <fost/internet>

#include <boost/filesystem/fstream.hpp>


using namespace fostlib;


FSL_MAIN(
    L"fget",
    L"Simple HTTP client\nCopyright (C) 2008-2009, Felspar Co. Ltd."
)( fostlib::ostream &o, fostlib::arguments &args ) {
    args.commandSwitch("socks", "Network settings", "Socks version");

    // The URL to be fetched (default to localhost)
    string location = args[ 1 ].value( L"http://localhost/" );
    o << location << std::endl;
    // Create a user agent and request the URL
    http::user_agent browser;
    std::auto_ptr< http::user_agent::response > response( browser.get( url( location ) ) );
    if ( args[2].isnull() ) {
        // Display the body
        o << response->body() << std::endl;
    } else {
        boost::filesystem::ofstream file(
            coerce< boost::filesystem::wpath >(args[2].value()), std::ios::binary
        );
        for (
            mime::const_iterator chunk( response->body().begin() );
            chunk != response->body().end(); ++chunk
        ) {
            const char
                *first = reinterpret_cast< const char * >((*chunk).first),
                *second = reinterpret_cast< const char * >((*chunk).second);
            file.write(first, second - first);
        }
    }
    return 0;
}
