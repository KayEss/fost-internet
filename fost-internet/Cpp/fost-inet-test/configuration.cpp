/*
    Copyright 2011, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet-test.hpp"


using namespace fostlib;


const setting< string > c_pop3_test_account(
    "fost-inet/Cpp/fost-inet-test/pop3.cpp",
    "POP3 client test", "Username",
    "pop3test@felspar.net", true);

const setting< string > c_smtp_host(
    "fost-internet/Cpp/fost-inet-tests/smtp.cpp",
    "fost-internet tests", "SMTP server",
    "localhost", true);
