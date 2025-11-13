#pragma once


#include <fost/core>
#include <fost/ua/cache.hpp>


namespace fostlib::ua {


    // The exception type that is thrown when there are no expectations
    /// left for the request and no cache exists and no HTTP requests can
    /// be made.
    class no_expectation : public fostlib::exceptions::exception {
      public:
        no_expectation(
                felspar::u8view message,
                felspar::u8view method,
                url const &url,
                std::optional<json> body,
                headers const &headers,
                std::source_location const & = std::source_location::current());

        felspar::u8view message() const noexcept override;
    };


    /// Generic status error exception
    class http_error : public fostlib::exceptions::exception {
      public:
        http_error(
                url const &u,
                std::source_location const & = std::source_location::current());
        http_error(
                url const &u,
                int status_code,
                std::source_location const & = std::source_location::current());

        felspar::u8view message() const noexcept override;
    };


    /// The resource could not be found.
    class resource_not_found : public http_error {
      public:
        resource_not_found(
                fostlib::url const &,
                std::source_location const & = std::source_location::current());

        felspar::u8view message() const noexcept override;
    };


    /// The user is not authorized yet (401)
    class unauthorized : public http_error {
      public:
        unauthorized(
                fostlib::url const &,
                std::source_location const & = std::source_location::current());

        felspar::u8view message() const noexcept override;
    };


    /// The resource is forbidden (403)
    class forbidden : public http_error {
      public:
        forbidden(
                fostlib::url const &,
                std::source_location const & = std::source_location::current());

        felspar::u8view message() const noexcept override;
    };


}
