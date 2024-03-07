#include "fost-inet.hpp"

#include <fost/http.useragent.hpp>
#include <fost/parse/http.client.hpp>

#include <fost/exception/parse_error.hpp>
#include <fost/exception/not_null.hpp>
#include <fost/exception/unexpected_eof.hpp>

#include <fost/datetime>
#include <fost/insert>
#include <fost/log>


/**
    ## fostlib::http::user_agent
*/


namespace {
    boost::asio::io_service g_io_service;

    const fostlib::setting<fostlib::string> c_user_agent(
            "fost-internet/Cpp/fost-inet/http.useragent.cpp",
            "HTTP",
            "UserAgent",
            "Fost user agent",
            true);
}


fostlib::http::user_agent::user_agent() {}
fostlib::http::user_agent::user_agent(const url &u) : base(u) {}


std::unique_ptr<fostlib::http::user_agent::response>
        fostlib::http::user_agent::operator()(request &req) const {
    try {
        if (!req.headers().exists("Date")) {
            req.headers().set(
                    "Date",
                    coerce<string>(coerce<rfc1123_timestamp>(std::chrono::system_clock::now())));
        }
        if (!req.headers().exists("Host")) {
            req.headers().set("Host", req.address().server().name());
        }
        if (!req.headers().exists("User-Agent")) {
            req.headers().set("User-Agent", c_user_agent.value() + "/Fost 4");
        }
        req.headers().set("TE", "trailers");

        if (authentication()) authentication()(req);

        network_connection cnx(req.address().server(), req.address().port());
        if (req.address().protocol() == "https") {
            cnx.start_ssl(req.address().server().name());
        }

        std::stringstream buffer;
        buffer << coerce<utf8_string>(req.method()).underlying() << " "
               << req.address().pathspec().underlying().underlying();
        {
            nullable<ascii_printable_string> q =
                    req.address().query().as_string();
            if (q) { buffer << "?" << q.value().underlying(); }
        }
        req.headers().fold_limit(null); // Turn off line folding
        buffer << " HTTP/1.0\r\n" << req.headers() << "\r\n";
        cnx << buffer;

        for (mime::const_iterator i(req.data().begin()); i != req.data().end();
             ++i) {
            cnx << *i;
        }

        utf8_string first_line;
        cnx >> first_line;
        response_status status;
        {
            auto pos = first_line.begin(), end = first_line.end();
            client_first_line<utf8_string::const_iterator> rule;
            if (not boost::spirit::qi::parse(pos, end, rule, status)
                || pos != end) {
                throw exceptions::parse_error(
                        "Expected a HTTP response", coerce<string>(first_line));
            }
        }

        return std::unique_ptr<http::user_agent::response>(
                new http::user_agent::response(
                        std::move(cnx), req.method(), req.address(),
                        std::move(status.version), status.status,
                        std::move(status.message)));
    } catch (fostlib::exceptions::exception &e) {
        insert(e.data(), "http-ua", "method", req.method());
        throw;
    }
}


/**
    ## fostlib::http::user_agent::request
*/


fostlib::http::user_agent::request::request(const string &method, const url &url)
: m_data{std::make_shared<empty_mime>()}, method(method), address(url) {}
fostlib::http::user_agent::request::request(
        const string &method,
        const url &url,
        const string &data,
        mime::mime_headers headers)
: m_data{std::make_shared<text_body>(data, std::move(headers))},
  method(method),
  address(url) {}
fostlib::http::user_agent::request::request(
        const string &method, const url &url, const std::filesystem::path &data)
: m_data{std::make_shared<file_body>(data)}, method(method), address(url) {}
fostlib::http::user_agent::request::request(
        const string &method, const url &url, std::shared_ptr<mime> mime_data)
: m_data{std::move(mime_data)}, method(method), address(url) {}


/**
    ## fostlib::http::user_agent::response
*/


fostlib::http::user_agent::response::response(
        const string &method,
        const url &address,
        int status,
        std::shared_ptr<binary_body> body,
        const mime::mime_headers &headers,
        const string &message)
: m_headers(headers),
  method(method),
  address(address),
  protocol(coerce<string>(address.protocol())),
  status(status),
  message(message),
  m_body(body) {}


namespace {
    void read_headers(
            fostlib::network_connection &cnx,
            fostlib::mime::mime_headers &headers,
            fostlib::nliteral error_message) {}
}


fostlib::http::user_agent::response::response(
        network_connection &&connection,
        const string &method,
        const url &url,
        const string &protocol,
        int status,
        const string &message)
: method(method),
  address(url),
  protocol(protocol),
  status(status),
  message(message),
  m_cnx(new network_connection(std::move(connection))) {
    try {
        while (true) {
            utf8_string line;
            (*m_cnx) >> line;
            if (line.empty()) break;
            m_headers.parse(coerce<string>(line));
        }
    } catch (fostlib::exceptions::exception &e) {
        insert(e.data(), "whilst", "fetching headers");
        insert(e.data(), "headers", m_headers);
        insert(e.data(), "status", status);
        insert(e.data(), "protocol", protocol);
        insert(e.data(), "method", method);
        throw;
    }
}


std::shared_ptr<fostlib::binary_body>
        fostlib::http::user_agent::response::body() {
    if (!m_body) {
        try {
            auto logger = fostlib::log::debug(c_fost_inet);
            logger("response", "status",
                   status())("response", "headers", headers());
            nullable<int64_t> length;
            if (method() == "HEAD") {
                length = 0;
            } else if (m_headers.exists("Content-Length")) {
                length = coerce<int64_t>(m_headers["Content-Length"].value());
            }
            logger("length", length);

            if (status() == 304 || (length && length.value() == 0)) {
                m_body = std::shared_ptr<binary_body>(
                        new binary_body(m_headers));
            } else if (not length) {
                if (m_headers["Transfer-Encoding"].value() == "chunked") {
                    logger("fetching", "transfer", "chunked");
                    std::vector<unsigned char> data;
                    while (true) {
                        std::string length, ignore_crlf;
                        *m_cnx >> length;
                        std::size_t chunk_size = fostlib::coerce<std::size_t>(
                                hex_string(length));
                        if (chunk_size == 0) break;
                        std::vector<unsigned char> chunk(chunk_size);
                        *m_cnx >> chunk >> ignore_crlf;
                        if (!ignore_crlf.empty())
                            throw fostlib::exceptions::not_null(
                                    "Expected CRLF after chunk data, but found "
                                    "something else",
                                    coerce<string>(utf8_string(ignore_crlf)));
                        data.insert(data.end(), chunk.begin(), chunk.end());
                    }
                    // Read trailing headers
                    read_headers(
                            *m_cnx, m_headers,
                            "Whilst reading trailing headers");
                    m_body = std::shared_ptr<binary_body>(
                            new binary_body(data, m_headers));
                } else {
                    logger("fetching", "transfer", "unknown size");
                    boost::asio::streambuf body_buffer;
                    *m_cnx >> body_buffer;
                    logger("fetching", "data", "size", body_buffer.size());
                    std::vector<unsigned char> body_data;
                    body_data.reserve(body_buffer.size());
                    while (body_buffer.size()) {
                        body_data.push_back(body_buffer.sbumpc());
                    }
                    m_body = std::shared_ptr<binary_body>(
                            new binary_body(body_data, m_headers));
                }
            } else {
                std::vector<unsigned char> body(length.value());
                *m_cnx >> body;
                m_body = std::shared_ptr<binary_body>(
                        new binary_body(body, m_headers));
            }
        } catch (fostlib::exceptions::exception &) { throw; }
    }
    return m_body;
}
