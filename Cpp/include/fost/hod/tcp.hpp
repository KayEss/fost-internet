/**
    Copyright 2016-2019, Felspar Co Ltd. <http://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once


#include <fost/internet>
#include <fost/hod/counters.hpp>
#include <fost/hod/connection.hpp>
#include <fost/hod/packet.hpp>

#include <boost/asio/spawn.hpp>


namespace fostlib {
    namespace hod {


        /// TCP specialisation of the decoder
        using tcp_decoder = decoder<socket_type>;


        /// TCP connection
        class tcp_connection : public connection<socket_type> {
          protected:
            tcp_connection(io_context_type &ios, peering p)
            : connection<socket_type>(p), socket(ios) {}

            io_context_type &get_io_service() override {
#if BOOST_VERSION >= 107000 // 1.70.0
                return socket.get_executor()
                    .template target<io_context_type::executor_type>()
                    ->context();
#else
                return socket.get_io_context();
#endif
            }

          public:
            /// The socket for this connection
            socket_type socket;
        };

        /// Connect to a remote end point over TCP
        template<typename Cnx, typename... A>
        std::shared_ptr<Cnx> tcp_connect(
                const fostlib::host &to,
                io_context_type &ios,
                A &&... a) {
            auto cnx = std::make_shared<Cnx>(ios, std::forward<A>(a)...);
            /// Try to connect to the remote server
            boost::asio::ip::tcp::resolver resolver{ios};
            boost::asio::ip::tcp::resolver::query q(
                    to.name().c_str(), to.service().value().c_str());
            boost::asio::ip::tcp::resolver::iterator endp = resolver.resolve(q),
                                                     end;
            boost::system::error_code error;
            while (endp != end) {
                cnx->socket.connect(*endp, error);
                if (error) {
                    fostlib::log::error(c_hod_tcp)(
                            "", "Connect error to endpoint")("error", error);
                    ++endp;
                } else {
                    cnx->process(cnx);
                    return cnx;
                }
            }
            throw fostlib::exceptions::connect_failure(
                    error, to, fostlib::coerce<uint16_t>(to.service().value()));
        }


        /// A loop implementation for receiving the inbound packets. The
        /// signature for the Dispatch handler (lambda) is:
        ///     (tcp_decoder decoder, uint8_t control, std::size_t bytes) -> void
        /// Note that the `tcp_decoder` is moved into the function.
        template<typename Dispatch>
        inline void receive_loop(
                tcp_connection &cnx,
                boost::asio::yield_context yield,
                Dispatch dispatch) {
            while (cnx.socket.is_open()) {
                try {
                    decoder<socket_type> decode(
                            cnx.socket, yield);
                    std::size_t packet_size = decode.read_size();
                    control_byte control = decode.read_byte();
                    decode.transfer(packet_size);
                    fostlib::log::debug(c_hod_tcp)("", "Got packet")(
                            "connection", cnx.id)("control", control)(
                            "size", "specified",
                            packet_size)("size", "buffered", decode.size());
                    dispatch(std::move(decode), control, packet_size);
                } catch (boost::coroutines::detail::forced_unwind &) {
                    throw;
                } catch (fostlib::exceptions::exception &e) {
                    cnx.socket.close();
                    fostlib::log::error(c_hod_tcp)(
                            "", "Socket error - exception caught")(
                            "connection", cnx.id)(
                            "exception", fostlib::coerce<fostlib::json>(e));
                } catch (std::exception &e) {
                    cnx.socket.close();
                    fostlib::log::error(c_hod_tcp)(
                            "", "Socket error - exception caught")(
                            "connection",
                            cnx.id)("exception", "what", e.what());
                } catch (...) {
                    cnx.socket.close();
                    fostlib::log::error(c_hod_tcp)(
                            "", "Socket error - exception caught")(
                            "connection", cnx.id);
                }
            }
            fostlib::log::info(c_hod_tcp)("", "Connection closed")(
                    "connection", cnx.id);
        }


    }
}


/// Implementation of TCP data send for outbound packets
template<>
inline void fostlib::hod::out_packet::operator()(
        fostlib::socket_type &sock,
        boost::asio::yield_context yield) const {
    boost::asio::streambuf header;
    size_sequence(size(), header);
    header.sputc(control);
    std::array<boost::asio::streambuf::const_buffers_type, 2> data{
            {header.data(), buffer->data()}};
    boost::system::error_code error;
    async_write(sock, data, yield[error]);
    if (not error) {
        ++p_sent;
    } else {
        sock.close();
        throw fostlib::exceptions::not_implemented(__func__, error);
    }
}


/// Implementation for transfer for TCP
template<>
inline void fostlib::hod::decoder<fostlib::socket_type>::transfer(
        std::size_t bytes) {
    /// If we have a socket then we transfer bytes for it. If however there
    /// is no socket this simply means that we assume that the input buffer
    /// already contains the data so we do nothing.
    if (socket) {
        boost::system::error_code error;
        boost::asio::async_read(
                *socket, *input_buffer, boost::asio::transfer_exactly(bytes),
                (*yield)[error]);
        if (error) {
            throw fostlib::exceptions::unexpected_eof(
                    "Reading bytes from socket", error);
        }
    }
}
