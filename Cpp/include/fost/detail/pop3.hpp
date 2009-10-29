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


        /// Iterates through all of the emails in a POP3 mailbox
        FOST_INET_DECLSPEC void iterate_mailbox(
            const host &host,
            boost::function<bool (const text_body &)> destroy_message,
            const string &username,
            const string &password
        );


        /// Returns true if the email message is a bounce delivery report (NDR)
        FOST_INET_DECLSPEC bool email_is_an_ndr( const text_body & );


    }


}


#endif // FOST_DETAIL_POP3_HPP
