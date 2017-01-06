/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/rask/packet.hpp>


namespace fostlib {


    /// Define a concrete protocl based on the Rask meta-protocol. Typically
    /// there will be a single, constant, global instance of this that descrbes
    /// what happens in response to each of the different control bytes that
    /// may be received.
    class rask_protocol {
    public:
        /// TODO: Version configuration
    };


}

