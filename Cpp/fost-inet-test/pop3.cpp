/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/

#include "fost-inet-test.hpp"
#include <fost/detail/pop3.hpp>


using namespace fostlib::pop3;
using namespace fostlib;


namespace {

    const setting< string > c_username("fost-inet/Cpp/fost-inet-test/pop3.cpp",
        "POP3 client test", "Username", "pop3test@felspar.net", true
    );

}


FSL_TEST_SUITE( pop3 );

bool destroy_message(const message &the_message) {
    return false;
};

FSL_TEST_FUNCTION( download_messages ) {
    host host(L"imap.felspar.net");

    iterate_mailbox(
        host,
        &message::bounced,
        coerce<utf8string>(c_username.value()),
        coerce<utf8string>(setting<string>::value(L"POP3 client test", L"Password"))
    );
}
