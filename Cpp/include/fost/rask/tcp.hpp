/*
    Copyright 2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>


namespace fostlib {


    class rask_tcp {
    public:
        /// Default construct the connection
        rask_tcp();

        /// The connection ID used in log messages
        const int64_t id;
    };


}

