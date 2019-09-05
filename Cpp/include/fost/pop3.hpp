/**
    Copyright 2009-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#ifndef FOST_DETAIL_POP3_HPP
#define FOST_DETAIL_POP3_HPP
#pragma once

#include <fost/string>
#include <fost/host.hpp>
#include <fost/connection.hpp>
#include <fost/smtp.hpp>
#include <fost/mime.hpp>


namespace fostlib {


    namespace pop3 {


        /// Iterates through all of the emails in a POP3 mailbox
        FOST_INET_DECLSPEC void iterate_mailbox(
                const host &host,
                std::function<bool(const text_body &)> destroy_message,
                const string &username,
                const string &password);


    }


}


#endif // FOST_DETAIL_POP3_HPP
