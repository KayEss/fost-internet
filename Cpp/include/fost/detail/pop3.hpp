/*
    Copyright 2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/

#ifndef FOST_DETAIL_POP3_HPP
#define FOST_DETAIL_POP3_HPP
#pragma once

#include <fost/string>
#include <fost/detail/host.hpp>
#include <fost/detail/connection.hpp>
#include <fost/detail/smtp.hpp>

namespace fostlib {

    namespace pop3 {
        
        typedef const size_t message_id;
        class FOST_INET_DECLSPEC message {
            std::map<string, string> m_headers;// need a headers class
            utf8string m_content;
            utf8string m_status;
        public:
            message(
                network_connection &the_network_connection
            );
            
            bool bounced() const;
        };
        
        void iterate_mailbox(
            const host &host,
            boost::function<bool (const message &)> destroy_message,
            const utf8string &username,
            const utf8string &password
        );
    }
    
}


#endif // FOST_DETAIL_POP3_HPP

