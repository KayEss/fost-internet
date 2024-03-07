#ifndef FOST_HOST_HPP
#define FOST_HOST_HPP
#pragma once


#include <fost/string>

#include <boost/asio.hpp>


namespace fostlib {


    typedef uint16_t port_number;


    /// An internet host with port information
    class FOST_INET_DECLSPEC host {
      public:
        host();
        explicit host(const string &host);
        host(const string &host, const nullable<string> &service);
        host(const string &host, port_number service);
        explicit host(uint32_t ipv4, const nullable<string> &service = null);
        host(uint8_t,
             uint8_t,
             uint8_t,
             uint8_t,
             const nullable<string> &service = null);

        boost::asio::ip::address address() const;
        string name() const;
        accessors<nullable<string>> service;

      private:
        fostlib::string m_name;
    };


    namespace exceptions {


        /// Thrown when a host name cannot be resolved
        class FOST_INET_DECLSPEC host_not_found : public exception {
          public:
            host_not_found(
                    const string &host_name,
                    felspar::source_location const & =
                            felspar::source_location::current()) noexcept;

          protected:
            felspar::u8view message() const noexcept override;
        };


    }


    /// Allow a host to be coerced (parsed) from a string
    template<>
    struct FOST_INET_DECLSPEC coercer<host, string> {
        /// Perform the coercion
        host coerce(const string &h);
    };
    /// Allow a host to be turned into an ASCII string representation
    template<>
    struct FOST_INET_DECLSPEC coercer<ascii_string, host> {
        /// Perform the coercion
        ascii_string coerce(const host &i);
    };
    /// Allow a host to be turned into a string
    template<>
    struct FOST_INET_DECLSPEC coercer<string, host> {
        /// Perform the coercion
        string coerce(const host &h) {
            return fostlib::coerce<string>(fostlib::coerce<ascii_string>(h));
        }
    };

    /// Allow a host to be coerced (parsed) from JSON
    template<>
    struct FOST_INET_DECLSPEC coercer<host, json> {
        /// Perform the coercion
        host coerce(const json &h) {
            return fostlib::coerce<host>(fostlib::coerce<string>(h));
        }
    };
    /// Allow a host to be turned into JSON
    template<>
    struct FOST_INET_DECLSPEC coercer<json, host> {
        /// Perform the coercion
        json coerce(const host &h) { return json(fostlib::coerce<string>(h)); }
    };

    /// Allow an IP number to be turned into a string
    template<>
    struct FOST_INET_DECLSPEC coercer<string, boost::asio::ip::address> {
        /// Perform the coercion
        string coerce(const boost::asio::ip::address &i);
    };


}


namespace std {


    /// Allow hosts to be output to a stream
    inline fostlib::ostream &
            operator<<(fostlib::ostream &o, const fostlib::host &h) {
        return o << fostlib::coerce<fostlib::string>(h);
    }


}


#endif // FOST_HOST_HPP
