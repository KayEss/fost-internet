/**
    Copyright 2017-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/hod/hod.hpp>

#include <boost/range.hpp> // Works around a bug in Boost 1.72.0
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>


namespace fostlib {
    namespace hod {


        /// A mechanism for decoding the two basic parts of the Rask
        /// protocol -- data and a size control sequence.

        /// These instances are created inside the transport's `receive_loop`
        /// where data is transferred from the socket and into the buffer.
        /// The dispatch process takes the decoder after all of the remaining
        /// bytes have been transferred.
        template<typename Transport>
        class decoder final {
            std::unique_ptr<boost::asio::streambuf> input_buffer;
            Transport *socket;
            std::unique_ptr<boost::asio::yield_context> yield;

            /// This is only movable
            decoder(const decoder &) = delete;
            decoder &operator=(const decoder &) = delete;

          public:
            /// Construct a decoder that starts by talking to the underlying
            /// transport for data.
            explicit decoder(Transport &s, boost::asio::yield_context y)
            : input_buffer(std::make_unique<boost::asio::streambuf>()),
              socket(&s),
              yield(std::make_unique<boost::asio::yield_context>(y)) {}
            /// Construct from an input_buffer that is handed over. This is
            /// useful for testing.
            explicit decoder(std::unique_ptr<boost::asio::streambuf> b)
            : input_buffer(std::move(b)), socket(nullptr) {}

            /// When the decoder is moved only the buffer comes along
            decoder(decoder &&d) : decoder(std::move(d.input_buffer)) {}

            /// Transfer bytes from the transport and into the buffer. If the
            /// `socket` is empty then do nothing.
            void transfer(std::size_t bytes);

            /// The number of bytes in the buffer
            std::size_t size() const { return input_buffer->size(); }

            /// Read a size sequence
            std::size_t read_size() {
                std::size_t size = read_byte();
                if (size > 0xf8) {
                    const int bytes = size - 0xf8;
                    size = 0u;
                    for (auto i = 0; i != bytes; ++i) {
                        size = (size << 8) + read_byte();
                    }
                } else if (size >= 0x80) {
                    throw fostlib::exceptions::not_implemented(
                            __func__, "Not a size control sequence",
                            fostlib::coerce<fostlib::string>(size));
                }
                return size;
            }

            /// Return a single byte of data
            unsigned char read_byte() {
                transfer(1u);
                return input_buffer->sbumpc();
            }

            /// Read data into array
            std::size_t read_data(char *into, std::size_t size) {
                transfer(size);
                return input_buffer->sgetn(into, size);
            }

            /// Read data
            template<typename C>
            std::size_t read_data(C &into, std::size_t bytes);
        };


    }
}
