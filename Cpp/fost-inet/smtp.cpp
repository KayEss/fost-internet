#include "fost-inet.hpp"
#include <fost/connection.hpp>
#include <fost/smtp.hpp>
#include <fost/parse/email.hpp>

#include <fost/exception/null.hpp>
#include <fost/exception/parse_error.hpp>

#include <fost/insert>


const fostlib::setting<fostlib::string> fostlib::c_smtp_host(
        "fost-internet/Cpp/fost-inet/smtp.cpp",
        "fost-internet tests",
        "SMTP server",
        "localhost",
        true);

const fostlib::setting<fostlib::port_number> fostlib::c_smtp_port(
        "fost-internet/Cpp/fost-inet/smtp.cpp",
        "fost-internet tests",
        "SMTP port",
        25,
        true);


/**
    fostlib::rfc822_address_tag
*/


void fostlib::rfc822_address_tag::do_encode(
        fostlib::nliteral from, ascii_printable_string &into) {
    throw exceptions::not_implemented(
            "fostlib::rfc822_address_tag::do_encode( fostlib::nliteral from, "
            "ascii_string &into )");
}
void fostlib::rfc822_address_tag::do_encode(
        const ascii_printable_string &from, ascii_printable_string &into) {
    throw exceptions::not_implemented(
            "fostlib::rfc822_address_tag::do_encode( const ascii_string &from, "
            "ascii_string &into )");
}
void fostlib::rfc822_address_tag::check_encoded(const ascii_printable_string &s) {
    if (s.empty()) throw exceptions::null{"Email address is empty"};
    if (s.underlying().find('@') == string::npos)
        throw exceptions::parse_error{
                "Email address doesn't contain @ symbol", coerce<string>(s)};
}


/**
    fostlib::email_address
*/


fostlib::email_address::email_address() {}

fostlib::email_address::email_address(
        const rfc822_address &address, const nullable<string> &name)
: email(address), name(name) {}

fostlib::email_address::email_address(
        const ascii_printable_string &address, const nullable<string> &name)
: email(rfc822_address(address)), name(name) {}


fostlib::string
        fostlib::coercer<fostlib::string, fostlib::email_address>::coerce(
                fostlib::email_address const &e) {
    if (not e.name()) {
        return "<" + fostlib::coerce<string>(e.email().underlying()) + ">";
    } else {
        return e.name().value() + " <"
                + fostlib::coerce<string>(e.email().underlying()) + ">";
    }
}
fostlib::email_address
        fostlib::coercer<fostlib::email_address, fostlib::string>::coerce(
                fostlib::string const &s) {
    std::pair<boost::optional<std::string>, std::string> result;
    smtp_address_parser<string::const_iterator> rule;
    auto pos = s.begin();
    if (boost::spirit::qi::parse(pos, s.end(), rule, result)
        && pos == s.end()) {
        if (not result.first)
            return rfc822_address(
                    fostlib::coerce<ascii_printable_string>(result.second));
        else
            return email_address(
                    rfc822_address(fostlib::coerce<ascii_printable_string>(
                            result.second)),
                    trim(string(result.first.value())));
    } else {
        throw exceptions::parse_error(
                "Could not parse email address", string(pos, s.end()));
    }
}


/*
    fostlib::smtp_client
*/


struct fostlib::smtp_client::implementation {
    bool can_send;
    network_connection cnx;

    implementation(const host &h, const port_number p)
    : can_send(false), cnx(h, p) {
        check(220, "Initial connection");
        cnx << "HELO FSIP\r\n";
        check(250, "HELO");
        can_send = true;
    }
    ~implementation() {
        if (can_send) {
            cnx << "QUIT\r\n";
            check(221, "QUIT");
        }
    }

    void check(int code, const string &command) {
        utf8_string number = coerce<utf8_string>(coerce<string>(code));
        utf8_string response;
        cnx >> response;
        if (not response.underlying().starts_with(number)) {
            exceptions::not_implemented exception(
                    "SMTP response was not the one expected");
            insert(exception.data(), "code", "expected", code);
            insert(exception.data(), "code", "received", response);
            insert(exception.data(), "command", command);
            insert(exception.data(), "response", response);
            throw exception;
        }
    }
};


fostlib::smtp_client::smtp_client(const host &server, const port_number p)
: m_impl(new implementation(server, p)) {}

fostlib::smtp_client::~smtp_client() try { delete m_impl; } catch (...) {
    absorb_exception();
}


void fostlib::smtp_client::send(
        const mime &email,
        const rfc822_address &to,
        const rfc822_address &from) {
    m_impl->can_send = false;

    m_impl->cnx << "MAIL FROM:<" + from.underlying().underlying() + ">\r\n";
    m_impl->check(250, "MAIL FROM");
    m_impl->cnx << "RCPT TO:<" + to.underlying().underlying() + ">\r\n";
    m_impl->check(250, "RCPT TO");
    m_impl->cnx << "DATA\r\n";
    m_impl->check(354, "DATA");

    std::stringstream ss;
    ss << email.headers();
    ss << "\r\n";
    m_impl->cnx << ss;

    for (mime::const_iterator d(email.begin()); d != email.end(); ++d)
        m_impl->cnx << *d;
    m_impl->cnx << "\r\n.\r\n";
    m_impl->check(250, "Data spooling");

    m_impl->can_send = true;
}
