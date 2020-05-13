/**
    Copyright 2009-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include "fost-inet-test.hpp"
#include <fost/pop3.hpp>


using namespace fostlib::pop3;
using namespace fostlib;


FSL_TEST_SUITE(pop3);


FSL_TEST_FUNCTION(download_messages) {
    host host(c_pop3_server.value());
    std::size_t mail_count = 0;

    FSL_CHECK_NOTHROW(iterate_mailbox(
            host, [&mail_count](const auto &) { return ++mail_count > 10; },
            c_pop3_test_account.value(),
            setting<string>::value("POP3 client test", "Password")));
}


FSL_TEST_FUNCTION(sending_tests) {
    host host(c_smtp_host.value());
    port_number port(c_smtp_port.value());

    smtp_client server(host, port);

    text_body mail(
            "This message shows that messages can be sent from "
            "appservices.felspar.com");
    mail.headers().set("Subject", "Test email -- send directly via SMTP");
    FSL_CHECK_NOTHROW(server.send(
            mail, "pop3test@felspar.com", "appservices@felspar.com"));

    text_body should_be_bounced(
            "This should be a bounced message. It shows that bounce messages "
            "are being received.");
    mail.headers().set("Subject", "Test email -- sent to invalid address");
    FSL_CHECK_NOTHROW(server.send(
            should_be_bounced, "not-valid@felspar.com", "pop3test@felspar.com"));
}
