/*
    Copyright 2009-2018, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/

#include "fost-inet.hpp"
#include <fost/pop3.hpp>

#include <regex>


using namespace fostlib;


bool fostlib::email_is_an_ndr(const text_body &email) {
    string subject = email.headers()["Subject"].value();
    return ( // Proper NDRs should have this Content-Type and contain a
             // recording of a 500 response
                   email.headers()["Content-Type"].subvalue("report-type")
                           == "delivery-status"
                   && (email.text().underlying().find("Status: 5")
                               != string::npos
                       || subject.find("Undeliverable: ") == 0))
            || (subject == "Delivery failure"
                || subject == "Delivery Notification: Delivery has failed"
                || subject == "Delivery Report (Failure)"
                || subject == "Delivery Status Notification(Failure)"
                || subject == "Delivery Status Notification (Failure)"
                || subject == "failure notice"
                || subject
                        == "Mail delivery failed: returning message to sender"
                || subject == "Undelivered Mail Returned to Sender")
            || (subject.find("DELIVERY FAILURE:") == 0
                || subject.find("Undeliverable: ") == 0);
}


namespace {
    const std::regex c_delay_regex("^Warning: message delayed \\d+ .+?$");
}
bool fostlib::email_is_a_delay_report(const text_body &email) {
    string subject = email.headers()["Subject"].value();
    return ( // Many don't have the correct report-type
             // email.headers()["Content-Type"].subvalue("report-type") ==
             // "delivery-status" &&
                   subject == "Delivery Status Notification (Delay)"
                   || subject == "Delivery Status Notification (Warning)")
            || std::regex_match(
                       coerce<utf8_string>(subject).underlying(),
                       c_delay_regex);
}


bool fostlib::email_is_out_of_office(const text_body &email) {
    string subject = email.headers()["Subject"].value();
    return (subject == "I'm out of the office"
            || subject.find("I am out of the office") == 0
            || subject.find(" is out of the office") != std::string::npos
            || subject.find("Out of Office") == 0
            || subject.find("Out of Office AutoReply: ") == 0
            || subject == "Vacation reply");
}
