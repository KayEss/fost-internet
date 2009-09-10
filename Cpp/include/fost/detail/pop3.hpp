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
#include <fost/detail/mime.hpp>



namespace fostlib {

    namespace pop3 {
        
        class FOST_INET_DECLSPEC message {
            mime::mime_headers m_headers;
            std::auto_ptr<text_body> m_text_body;
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

