/**
    Copyright 2011-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include "fost-inet-test.hpp"


using namespace fostlib;


const setting<string> c_smtp_host(
        "fost-internet/Cpp/fost-inet-tests/smtp.cpp",
        "fost-internet tests",
        "SMTP server",
        "localhost",
        true);

const setting<port_number> c_smtp_port(
        "fost-internet/Cpp/fost-inet-tests/smtp.cpp",
        "fost-internet tests",
        "SMTP port",
        25,
        true);

const setting<string> c_pop3_server(
        "fost-inet/Cpp/fost-inet-test/pop3.cpp",
        "fost-internet tests",
        "POP3 server",
        "imap.felspar.net",
        true);

const setting<string> c_pop3_test_account(
        "fost-inet/Cpp/fost-inet-test/pop3.cpp",
        "POP3 client test",
        "Username",
        "pop3test@felspar.net",
        true);
