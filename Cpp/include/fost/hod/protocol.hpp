/**
    Copyright 2017-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/hod/packet.hpp>


namespace fostlib {
    namespace hod {


        /// Define a concrete protocl based on the Rask meta-protocol. Typically
        /// there will be a single, constant, global instance of this that
        /// descrbes what happens in response to each of the different control
        /// bytes that may be received.
        template<typename P>
        class protocol : boost::noncopyable {
          public:
            /// The packet processing function type
            using packet_processor = P;

            /// Configure the versions
            template<typename D>
            protocol(
                    D vAny_default,
                    std::initializer_list<std::initializer_list<
                            std::pair<int, packet_processor>>> versions) {
                // Build the base array
                control_bytes base;
                for (int cc{}; cc != 256; ++cc) {
                    base[cc] = [vAny_default, cc](auto &&...v) {
                        vAny_default(cc, v...);
                    };
                }
                // Build the other versions
                for (auto &&version : versions) {
                    for (auto &&binding : version) {
                        base[binding.first] = binding.second;
                    }
                    processors.push_back(base);
                }
            }

            /// Return the maximum version supported by this end point.
            uint8_t max_version() const {
                return fostlib::coerce<uint8_t>(processors.size() - 1u);
            }

            /// Dispatch the message to the right handler
            template<typename... A>
            auto dispatch(uint8_t version, control_byte control, A &&...a) const {
                return processors.at(version)[control](std::forward<A>(a)...);
            }

          private:
            /// The control byte lookup table
            using control_bytes = std::array<packet_processor, 256>;
            /// The version lookup handler
            std::vector<control_bytes> processors;
        };


    }
}
