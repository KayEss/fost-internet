#include "fost-inet-test.hpp"


using namespace fostlib;


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
