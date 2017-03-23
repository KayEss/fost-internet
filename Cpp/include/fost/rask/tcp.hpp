/*
    Copyright 2016-2017, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/internet>
#include <fost/rask/counters.hpp>
#include <fost/rask/connection.hpp>
#include <fost/rask/packet.hpp>

#include <boost/asio/spawn.hpp>


namespace rask {


    /// Module for TCP
    extern const fostlib::module c_rask_proto_tcp;


    /// TCP specialisation of the decoder
    using tcp_decoder = decoder<boost::asio::ip::tcp::socket>;


    /// TCP connection
    class tcp_connection : public connection<boost::asio::ip::tcp::socket> {
    protected:
        tcp_connection(boost::asio::io_service &ios, peering p)
        : connection<boost::asio::ip::tcp::socket>(p), socket(ios) {
        }

        boost::asio::io_service &get_io_service() override {
            return socket.get_io_service();
        }

    public:
        /// The socket for this connection
        boost::asio::ip::tcp::socket socket;
    };


    /// Listen for a connection. When a connection is established the factory
    /// is used to create a connection object which is then expected to handle
    /// the ongoing connection.
    void tcp_listen(boost::asio::io_service &ios, fostlib::host netloc,
        std::function<std::shared_ptr<tcp_connection>(boost::asio::ip::tcp::socket)> factory);

    /// Connect to a remote end point over TCP
    template<typename Cnx, typename... A>
    std::shared_ptr<Cnx> tcp_connect(const fostlib::host &to,boost::asio::io_service &ios,  A&&... a) {
        auto cnx = std::make_shared<Cnx>(ios, std::forward<A>(a)...);
        /// Try to connect to the remote server
        boost::asio::ip::tcp::resolver resolver{ios};
        boost::asio::ip::tcp::resolver::query q(to.name().c_str(), to.service().value().c_str());
        boost::asio::ip::tcp::resolver::iterator endp = resolver.resolve(q), end;
        boost::system::error_code error;
        while ( endp != end ) {
            cnx->socket.connect(*endp, error);
            if ( error ) {
                fostlib::log::error(c_rask_proto)
                    ("", "Connect error to endpoint")
                    ("error", error);
                ++endp;
            } else {
                cnx->process(cnx);
                return cnx;
            }
        }
        throw fostlib::exceptions::connect_failure(error, to,
            fostlib::coerce<uint16_t>(to.service().value()));
    }


    /// A loop implementation for receiving the inbound packets. The
    /// signature for the Dispatch handler (lambda) is:
    ///     (tcp_decoder decoder, uint8_t control, std::size_t bytes)
    /// Note that the `tcp_decoder` is moved into the function.
    template<typename Dispatch> inline
    void receive_loop(
        tcp_connection &cnx, boost::asio::yield_context &yield, Dispatch dispatch
    ) {
        while ( cnx.socket.is_open() ) {
            try {
                rask::decoder<boost::asio::ip::tcp::socket> decode(cnx.socket, yield);
                std::size_t packet_size = decode.read_size();
                rask::control_byte control = decode.read_byte();
                decode.transfer(packet_size);
                fostlib::log::debug(c_rask_proto)
                    ("", "Got packet")
                    ("connection", cnx.id)
                    ("control", control)
                    ("size", "specified", packet_size)
                    ("size", "buffered", decode.size());
                dispatch(std::move(decode), control, packet_size);
            } catch ( boost::coroutines::detail::forced_unwind & ) {
                throw;
            } catch ( fostlib::exceptions::exception &e ) {
                cnx.socket.close();
                fostlib::log::error(c_rask_proto)
                    ("", "Socket error - exception caught")
                    ("connection", cnx.id)
                    ("exception", fostlib::coerce<fostlib::json>(e));
            } catch ( std::exception &e ) {
                cnx.socket.close();
                fostlib::log::error(c_rask_proto)
                    ("", "Socket error - exception caught")
                    ("connection", cnx.id)
                    ("exception", "what", e.what());
            } catch ( ... ) {
                cnx.socket.close();
                fostlib::log::error(c_rask_proto)
                    ("", "Socket error - exception caught")
                    ("connection", cnx.id);
            }
        }
        fostlib::log::info(c_rask_proto)
            ("", "Connection closed")
            ("connection", cnx.id);
    }


}


/// Implementation of TCP data send for outbound packets
template<> inline
void rask::out_packet::operator () (
    boost::asio::ip::tcp::socket &sock, boost::asio::yield_context &yield
) const {
    boost::asio::streambuf header;
    size_sequence(size(), header);
    header.sputc(control);
    std::array<boost::asio::streambuf::const_buffers_type, 2>
        data{{header.data(), buffer->data()}};
    boost::system::error_code error;
    async_write(sock, data, yield[error]);
    if ( not error ) {
        ++p_sent;
    } else {
        sock.close();
        throw fostlib::exceptions::not_implemented(__func__, error);
    }
}


/// Implementation for transfer for TCP
template<> inline
void rask::decoder<boost::asio::ip::tcp::socket>::transfer(std::size_t bytes) {
    /// If we have a socket then we transfer bytes for it. If however there
    /// is no socket this simply means that we assume that the input buffer
    /// already contains the data so we do nothing.
    if ( socket ) {
        boost::system::error_code error;
        boost::asio::async_read(*socket, *input_buffer,
            boost::asio::transfer_exactly(bytes), (*yield)[error]);
        if ( error ) {
            throw fostlib::exceptions::unexpected_eof("Reading bytes from socket", error);
        }
    }
}

