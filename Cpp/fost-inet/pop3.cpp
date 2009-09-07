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

/*void write_line(utf8string string) {
    std::cout << string;
    std::cout << std::endl;
}*/

message::message(
        network_connection &the_network_connection
    ) {
    std::list<string> headers; // need a headers class
    
    utf8string line;
    the_network_connection >> line;

/* read header lines: */
    while (
        ! line.empty()
    ) {
        utf8string header(line);
        
        line = "";
        the_network_connection >> line;
        
    /* read next header line, if starting with space, add it to header */
        while (
            (line.substr(0,1) == " ") ||
            (line.substr(0,1) == "\t")
        ) {
            header += line;
            line = "";
            the_network_connection >> line;
        }
        headers.push_back(header);
    };
    
/* read the body: */
    while ( 
       (line.substr(0,1) != ".") ||
       (line.substr(1,2) == ".")
    ) {
        m_content += line+"\n";        
        line = "";
        the_network_connection >> line;
    };
    //write_line(m_content);
}




void check_OK(network_connection &the_network_connection, utf8string command) {
    utf8string server_response;
    the_network_connection >> server_response;

    //write_line("Server: "+server_response);

    if (server_response.substr(0,3) != "+OK") {
        throw exceptions::not_implemented(
            command,
            server_response
        );
    };    
}

void send(
        network_connection &the_network_connection,
        const utf8string command, 
        const utf8string parameter
    ) {
    
//    write_line("Client: "+command+" "+parameter);

    {
        the_network_connection << command;
        the_network_connection << " ";
        the_network_connection << parameter;
        the_network_connection << "\r\n";
    }
}

void send(
        network_connection &the_network_connection,
        const utf8string command
    ) {
//    write_line("Client: "+command);    

    the_network_connection << command;
    the_network_connection << "\r\n";
}

void send(
        network_connection &the_network_connection,
        const utf8string command, 
        const size_t parameter
    ) {    
    std::stringstream i_stream;
    i_stream << parameter;
    utf8string value(i_stream.str());

    send(the_network_connection, command, value);
}



void send_and_check(
        network_connection &the_network_connection,
        const utf8string command, 
        const utf8string parameter
    ) {
    send(the_network_connection, command, parameter);
    check_OK(the_network_connection, command);
}

void send_and_check(
        network_connection &the_network_connection,
        const utf8string command, 
        const size_t parameter
    ) {
    send(the_network_connection, command, parameter);
    check_OK(the_network_connection, command);    
}

void send_and_check(
        network_connection &the_network_connection,
        const utf8string command
    ) {
    send(the_network_connection, command);
    check_OK(the_network_connection, command);    
}



void pop3::iterate_mailbox(
        const host &host,
        boost::function<bool (const message &)> destroy_message,
        const utf8string &username,
        const utf8string &password
    ) {
    network_connection the_network_connection( host, 110 );
    
    utf8string server_status;    
    the_network_connection >> server_status;
    
    send_and_check(the_network_connection, "user", username);
    send_and_check(the_network_connection, "pass", password);

    send(the_network_connection, "stat");
    
    utf8string server_response;
    the_network_connection >> server_response;
//    write_line("SERVER: "+server_response);
    
    std::stringstream server_response_stringstream(server_response.substr(3));
    size_t message_count;
    server_response_stringstream >> message_count;
    size_t octets;
    server_response_stringstream >> octets;
    
//    write_line("messages: "+message_count);
    for (
        size_t i = 1;
        i <= message_count;
        ++i
    ) {
        send_and_check(the_network_connection, "retr", i);
        
        utf8string content;

        message message(
            the_network_connection
        );

        if ( destroy_message(message) ) {
            send_and_check(the_network_connection, "dele", i);
        }        
    }
}
