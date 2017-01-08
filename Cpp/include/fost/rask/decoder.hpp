/*
    Copyright 2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/rask/rask-proto.hpp>


namespace rask {


    class decoder_base {
    public:
        virtual std::size_t read_size() = 0;
        virtual char read_byte() = 0;
        virtual void read_data(char *into, std::size_t bytes) = 0;
    };


    /// A mechanism for decoding the two basic parts of the Rask
    /// protocol -- data and a size control sequence
    template<typename ReadByte, typename ReadBytes>
    class decoder final : public decoder_base, boost::noncopyable {
        ReadByte byte;
        ReadBytes bytes;

    public:
        decoder(ReadByte b, ReadBytes bs)
        : byte(std::move(b)), bytes(std::move(bs)) {
        }

        decoder(decoder &&d)
        : byte(std::move(d.byte)), bytes(std::move(d.bytes)) {
        }

        /// Read a size sequence
        std::size_t read_size() override {
            std::size_t size = byte();
            if ( size > 0xf8 ) {
                const int bytes = size - 0xf8;
                size = 0u;
                for ( auto i = 0; i != bytes; ++i ) {
                    size = (size << 8) + byte();
                }
            } else if ( size >= 0x80 ) {
                throw fostlib::exceptions::not_implemented(__func__, "Not a size control sequence");
            }
            return size;
        }

        /// Return a single byte of data
        char read_byte() override {
            return byte();
        }

        /// Read data into array
        void read_data(char *into, std::size_t size) override {
            bytes(into, size);
        }

        /// Read data
        template<typename C>
        std::size_t read_data(C &into, std::size_t bytes);
    };


    /// Create a decoder out of two lambda expressions
    template<typename RB, typename RBs>
    decoder<RB, RBs> make_decoder(RB b, RBs bs) {
        return decoder<RB, RBs>{b, bs};
    }


}

