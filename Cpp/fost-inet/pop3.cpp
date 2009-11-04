/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/

#include "fost-inet.hpp"
#include <fost/detail/pop3.hpp>
#include <fost/exception/out_of_range.hpp>


using namespace fostlib;
using namespace fostlib::pop3;


namespace {

    mime::mime_headers read_headers(
        network_connection &the_network_connection
    ) {
        mime::mime_headers headers;

        utf8_string line;
        the_network_connection >> line;

        while ( !line.empty() ) {
            utf8_string header(line);

            line = "";
            the_network_connection >> line;

            while (
                (line.underlying().substr(0,1) == " ") ||
                (line.underlying().substr(0,1) == "\t")
            ) {
                header += line;
                line = "";
                the_network_connection >> line;
            }

            headers.parse(coerce< string >(header));
        }
        return headers;
    }

    std::auto_ptr<text_body> read_body(
        const mime::mime_headers &headers,
        network_connection &the_network_connection
    ) {
        std::string content;
        std::string line;

        the_network_connection >> line;
        while (
            true
        ) {
            if (
                (line.length() > 0) &&
                (line[0] == '.')
            ) {
                if (
                    (line.length() > 1) &&
                    (line[1] == '.')
                ) {
                    line = line.substr(1);
                } else
                    break;
            } else
                content += line+"\n";

            line = "";
            the_network_connection >> line;
        }

        return std::auto_ptr<text_body>(
            new text_body(
                coerce< string >( utf8_string(content) ),
                headers,
                "text/plain"
            )
        );
    }


    void check_OK(
        network_connection &the_network_connection,
        string command
    ) {
        utf8_string server_response;
        the_network_connection >> server_response;

        if (server_response.underlying().substr(0,3) != "+OK") {
            throw exceptions::not_implemented(
                command,
                coerce< string >(server_response)
            );
        }
    }

    void send(
        network_connection &the_network_connection,
        const string command,
        const nullable< string > parameter = null
    ) {
        the_network_connection << coerce< utf8_string >(command);
        if ( !parameter.isnull() ) {
            the_network_connection << " ";
            the_network_connection << coerce< utf8_string >(parameter);
        }
        the_network_connection << "\r\n";
    }

    void send(
        network_connection &the_network_connection,
        const string command,
        const size_t parameter
    ) {
        std::stringstream i_stream;
        i_stream << parameter;
        string value(coerce<string>(utf8_string(i_stream.str())));

        send(the_network_connection, command, value);
    }



    void send_and_check_OK(
        network_connection &the_network_connection,
        const string command,
        const string parameter
    ) {
        send(the_network_connection, command, parameter);
        check_OK(the_network_connection, command);
    }

    void send_and_check_OK(
        network_connection &the_network_connection,
        const string command,
        const size_t parameter
    ) {
        send(the_network_connection, command, parameter);
        check_OK(the_network_connection, command);
    }

    void send_and_check_OK(
        network_connection &the_network_connection,
        const string command
    ) {
        send(the_network_connection, command);
        check_OK(the_network_connection, command);
    }

}


namespace {
    class pop3cnx {
        network_connection m_cnx;
        public:
            size_t message_count;
            pop3cnx( const host &h, const string &username, const string &password )
            : m_cnx( h, 110 ) {
                utf8_string server_status;
                m_cnx >> server_status;

                send_and_check_OK(m_cnx, "user", username);
                send_and_check_OK(m_cnx, "pass", password);

                send(m_cnx, "stat");

                utf8_string server_response;
                m_cnx >> server_response;

                std::stringstream server_response_stringstream(server_response.underlying().substr(3));
                server_response_stringstream >> message_count;
                size_t octets;
                server_response_stringstream >> octets;
            }
            ~pop3cnx()
            try {
                send_and_check_OK(m_cnx, "quit");
            } catch ( ... ) {
                absorbException();
            }
            std::auto_ptr< text_body > message( size_t i ) {
                send_and_check_OK(m_cnx, "retr", i);
                return read_body(read_headers(m_cnx), m_cnx);
            }
            void remove( size_t i ) {
                send_and_check_OK(m_cnx, "dele", i);
            }
    };
}

void fostlib::pop3::iterate_mailbox(
    const host &host,
    boost::function<bool (const text_body &)> destroy_message,
    const string &username,
    const string &password
) {
    boost::scoped_ptr< pop3cnx > mailbox( new pop3cnx(host, username, password) );
    const size_t messages = mailbox->message_count;

    // Loop from the end so we always process the latest bounce messages first
    for ( std::size_t i = messages; i; --i ) {
        std::auto_ptr< text_body > message = mailbox->message(i);
        if (destroy_message(*message))
            mailbox->remove(i);
        if ( i % 20 == 0 ) {
            mailbox.reset( new pop3cnx(host, username, password) );
            if ( mailbox->message_count < i )
                throw fostlib::exceptions::out_of_range< size_t >(
                    "The number of messages on the server can't go down below the ones we've processed!",
                    i, messages, mailbox->message_count
                );
        }
    }
}


bool fostlib::pop3::email_is_an_ndr( const text_body &email ) {
    return
        ( // Proper NDRs should have this Content-Type and contain a recording of a 500 response
            email.headers()["Content-Type"].subvalue("report-type") == "delivery-status"
            && email.text().underlying().find("Status: 5") != string::npos
        )
    ;
}


bool fostlib::pop3::email_is_out_of_office( const text_body &email ) {
    string subject = email.headers()["Subject"].value();
    return
        (
            subject.find("Out of Office: ") == 0
            || subject.find("Out of Office AutoReply: ") == 0
        )
    ;
}
