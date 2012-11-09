/*
    Copyright 2009-2012, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/

#include "fost-inet-test.hpp"
#include <fost/pop3.hpp>


using namespace fostlib::pop3;
using namespace fostlib;


FSL_TEST_SUITE( pop3 );


namespace {
    bool delete_mails(std::size_t &number, const text_body &) {
        return ++number > 10;
    }
}
FSL_TEST_FUNCTION( download_messages ) {
    host host(c_pop3_server.value());
    std::size_t mail_count = 0;

    FSL_CHECK_NOTHROW(
        iterate_mailbox(host,
            boost::lambda::bind(delete_mails,
                boost::ref(mail_count), boost::lambda::_1),
            c_pop3_test_account.value(),
            setting<string>::value(
                L"POP3 client test",
                L"Password")));
}


FSL_TEST_FUNCTION( sending_tests ) {
    host host(c_smtp_host.value());
    port_number port(c_smtp_port.value());

    smtp_client server(host, port);

    text_body mail(
        L"This message shows that messages can be sent from appservices.felspar.com");
    mail.headers().set(L"Subject", L"Test email -- send directly via SMTP");
    FSL_CHECK_NOTHROW(
        server.send(mail,
            "pop3test@felspar.com", "appservices@felspar.com"));

    text_body should_be_bounced(
        L"This should be a bounced message. It shows that bounce messages "
        L"are being received.");
    mail.headers().set(L"Subject", L"Test email -- sent to invalid address");
    FSL_CHECK_NOTHROW(
        server.send(
            should_be_bounced, "not-valid@felspar.com", "pop3test@felspar.com"));
}
