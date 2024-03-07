#pragma once


#include <fost/ua/cache.hpp>


namespace fostlib::ua {


    /// Generate the hash from the URL and vary headers
    felspar::u8string cache_key(felspar::u8view method, url const &, headers const &);

    /// Determine if a method is idempotent or not. Note that even if a method
    /// is considered idempotent that does not mean that a particular request
    /// will be.
    bool idempotent(felspar::u8view);


}
