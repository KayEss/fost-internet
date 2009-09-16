/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/

#include "fost-inet.hpp"
#include <fost/detail/pop3.hpp>


#include <iostream>

using namespace fostlib::pop3;
using namespace fostlib;

namespace {
    void write_line(
        string string
    ) {
        std::cout << string;
        std::cout << std::endl;
    }
}

mime::mime_headers read_headers(
    network_connection &the_network_connection
) {
    mime::mime_headers m_headers;

    utf8string line;
    the_network_connection >> line;

    while (
        ! line.empty()
    ) {
        utf8string header(line);

        line = "";
        the_network_connection >> line;

        while (
           (line.substr(0,1) == " ") ||
           (line.substr(0,1) == "\t")
        ) {
            header += line;
            line = "";
            the_network_connection >> line;
        }

        m_headers.parse(coerce< string >(header));
    };
    return m_headers;
};

std::auto_ptr<text_body> read_body(
    const mime::mime_headers &m_headers,
    network_connection &the_network_connection
) {
    string content;
    utf8string line;
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
            }
            else {
                break;
            }
        }
        else {
            content += coerce<string>(line)+L"\n";
        }

        line = "";
        the_network_connection >> line;
    };

    std::auto_ptr<text_body> result(
        new text_body(
            content,
            m_headers,
            "text/plain"
        )
    );
    return result;
};

message::message(
    network_connection &the_network_connection
)
: m_headers(
    read_headers(
        the_network_connection
    )
), m_text_body(
    read_body(
        m_headers,
        the_network_connection
    )
) {}

bool message::bounced()
const {
/*
    nullable<string> report_type(m_headers["Content-Type"].subvalue("report-type"));

    if ( ! report_type.isnull() ) {
        std::cout << report_type.value() << "\n";
    }
    else {
        std::cout << "(Empty Content-Type)\n";
    }
*/

    bool bounced = 
        m_headers["Content-Type"].subvalue("report-type") == "delivery-status"
        && m_text_body->text().find("Status: 5") != string::npos
    ;     

    //std::cout << bounced;
    
    return bounced; 
};

namespace {
    void check_OK(
        network_connection &the_network_connection,
        string command
    ) {
        utf8string server_response;
        the_network_connection >> server_response;

        //write_line("Server: "+server_response);

        if (server_response.substr(0,3) != "+OK") {
            throw exceptions::not_implemented(
                command,
                coerce< string >(server_response)
            );
        };
    }

    void send(
        network_connection &the_network_connection,
        const string command,
        const nullable< string > parameter = null
    ) {
        //write_line("Client: "+command+" "+parameter);

        {
            the_network_connection << coerce< utf8string >(command);
            if ( ! parameter.isnull() ) {
                the_network_connection << " ";
                the_network_connection << coerce< utf8string >(parameter);
            }
            the_network_connection << "\r\n";
        }
    }

    void send(
        network_connection &the_network_connection,
        const string command,
        const size_t parameter
    ) {
        std::stringstream i_stream;
        i_stream << parameter;
        string value(coerce<string>(i_stream.str()));

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

void fostlib::pop3::iterate_mailbox(
    const host &host,
    boost::function<bool (const message &)> destroy_message,
    const string &username,
    const string &password
) {
    network_connection the_network_connection( host, 110 );

    utf8string server_status;
    the_network_connection >> server_status;

    send_and_check_OK(the_network_connection, "user", username);
    send_and_check_OK(the_network_connection, "pass", password);

    send(the_network_connection, "stat");

    utf8string server_response;
    the_network_connection >> server_response;
    //write_line("SERVER: "+server_response);

    std::stringstream server_response_stringstream(server_response.substr(3));
    size_t message_count;
    server_response_stringstream >> message_count;
    size_t octets;
    server_response_stringstream >> octets;

    //write_line("messages: "+message_count);
    for (
        size_t i = 1;
        i <= message_count;
        ++i
    ) {
        send_and_check_OK(the_network_connection, "retr", i);
        string content;
        message message(
            the_network_connection
        );

        if (
            destroy_message(message)
        ) {
            send_and_check_OK(the_network_connection, "dele", i);
        }
    }
    send_and_check_OK(the_network_connection, "quit");
}
