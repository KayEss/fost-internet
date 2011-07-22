/*
    Copyright 2009-2010, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/

#include "fost-inet-test.hpp"
#include <fost/pop3.hpp>


using namespace fostlib::pop3;
using namespace fostlib;


const setting< string > c_pop3_test_account(
    "fost-inet/Cpp/fost-inet-test/pop3.cpp",
    "POP3 client test",
    "Username",
    "pop3test@felspar.net",
    true
);


FSL_TEST_SUITE( pop3 );


FSL_TEST_FUNCTION( download_messages ) {
    host host(L"imap.felspar.net");

    FSL_CHECK_NOTHROW(
        iterate_mailbox(
            host,
            email_is_an_ndr,
            c_pop3_test_account.value(),
            setting<string>::value(
                L"POP3 client test",
                L"Password"
            )
        )
    );
}


FSL_TEST_FUNCTION( sending_tests ) {
    host host(c_smtp_host.value());

    smtp_client server( host );

    text_body mail(
        L"This message shows that messages can be sent from appservices.felspar.com"
    );
    mail.headers().set(L"Subject", L"Test email -- send directly via SMTP");
    FSL_CHECK_NOTHROW(
        server.send(mail, "pop3test@felspar.com", "appservices@felspar.com")
    );

    text_body should_be_bounced(
        L"This should be a bounced message. It shows that bounce messages "
        L"are being received."
    );
    mail.headers().set(L"Subject", L"Test email -- sent to invalid address");
    FSL_CHECK_NOTHROW(
        server.send(
            should_be_bounced, "not-valid@felspar.com", "pop3test@felspar.com"
        )
    );
}
