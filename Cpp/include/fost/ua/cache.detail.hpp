/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/ua/cache.hpp>


namespace fostlib::ua {


    /// Generate the hash from the URL and vary headers
    f5::u8string cache_key(f5::u8view method, url const &, headers const &);


}
