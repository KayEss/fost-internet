#include "fost-inet-test.hpp"
#include <fost/smtp.hpp>

#include <fost/exception/parse_error.hpp>


using namespace fostlib;


FSL_TEST_SUITE(smtp);


FSL_TEST_FUNCTION(basic) {
    email_address sample(rfc822_address("address@example.com"));
    sample = rfc822_address("sample@example.com");
}


FSL_TEST_FUNCTION(to_string) {
    email_address addy(rfc822_address("address@example.com"));
    FSL_CHECK_EQ(coerce<string>(addy), "<address@example.com>");
    addy.name("Mr. Address");
    FSL_CHECK_EQ(coerce<string>(addy), "Mr. Address <address@example.com>");
    FSL_CHECK_EQ(
            coerce<utf8_string>(addy), "Mr. Address <address@example.com>");
}

#define PARSE_PLAIN(s) \
    FSL_CHECK_EQ(coerce<email_address>(string(s)).email(), rfc822_address(s)); \
    FSL_CHECK(not coerce<email_address>(string(s)).name());
FSL_TEST_FUNCTION(from_string_plain) {
    PARSE_PLAIN("address@example.com");
    PARSE_PLAIN("address-whatever@example.com");
    PARSE_PLAIN("address+whatever@example.com");
    PARSE_PLAIN("address.whatever@example.com");
    FSL_CHECK_EXCEPTION(
            PARSE_PLAIN("<address-whatever@example.com>"),
            exceptions::parse_error &);
}

FSL_TEST_FUNCTION(smtp_send) {
    smtp_client server(host(c_smtp_host.value()), c_smtp_port.value());

    text_body mail("This is just a simple test email\n\nIgnore/delete it\n");
    mail.headers().set("Subject", "Test email");
    server.send(mail, "kirit@felspar.com", "pop3test@felspar.com");
}
