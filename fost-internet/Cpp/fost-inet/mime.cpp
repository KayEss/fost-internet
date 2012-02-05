/*
    Copyright 1999-2012, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/mime.hpp>
#include <fost/string>

#include <fost/exception/parse_error.hpp>
#include <fost/exception/unexpected_eof.hpp>

#include <boost/filesystem/fstream.hpp>


using namespace fostlib;


/*
    fostlib::mime
*/


fostlib::mime::mime( const mime_headers &h, const string &content_type )
: content_type( content_type ), headers( h ) {
    if ( !headers().exists("Content-Type") )
        headers().set(L"Content-Type", mime::mime_headers::content(content_type));
}

fostlib::mime::~mime() {
}


fostlib::mime::const_iterator fostlib::mime::begin() const {
    return const_iterator(iterator());
}
fostlib::mime::const_iterator fostlib::mime::end() const {
    return const_iterator(std::auto_ptr< fostlib::mime::iterator_implementation >());
}


/*
    fostlib::mime::const_iterator
*/


fostlib::mime::const_iterator::const_iterator( std::auto_ptr< fostlib::mime::iterator_implementation > i )
: underlying( i.release() ) {
    if ( underlying.get() )
        current = (*underlying)();
    else
        current = std::pair< const unsigned char *, const unsigned char * >();
}


const_memory_block fostlib::mime::const_iterator::operator * () const {
    return current;
}
const mime::const_iterator &fostlib::mime::const_iterator::operator ++() {
    if ( !underlying.get() )
        throw exceptions::unexpected_eof("fostlib::mime::const_iterator represents end of sequence");
    current = (*underlying)();
    return *this;
}


bool fostlib::mime::const_iterator::operator == ( const const_iterator &o ) const {
    return current == *o;
}
bool fostlib::mime::const_iterator::operator != ( const const_iterator &o ) const {
    return current != *o;
}


/*
    fostlib::mime::iterator_implementation
*/


fostlib::mime::iterator_implementation::~iterator_implementation() {
}


/*
    fostlib::mime::mime_headers
*/

std::pair< string, headers_base::content > fostlib::mime::mime_headers::value( const string &name, const string &value ) {
    if ( name == L"Content-Disposition" || name == L"content-disposition" || name == L"Content-Type" ) {
        std::map< string, string > args;
        // Parse the value from the format
        // form-data; name="aname"; extra="value"
        std::pair< string, nullable< string > > disp( partition( value, L";" ) );
        if ( !disp.second.isnull() ) {
            for ( std::pair< string, nullable< string > > para( partition( disp.second, L";" ) ); !para.first.empty(); para = partition( para.second, L";" ) ) {
                // Normally the extra argument values should be surrounded by double quotes, but sometimes not
                std::pair< string, nullable< string > > argument = partition( para.first, L"=" );
                if ( !argument.second.isnull()
                    && argument.second.value().at(0) == '"'
                    && argument.second.value().at(argument.second.value().length()-1) == '"'
                ) {
                    argument.second = argument.second.value().substr(1, argument.second.value().length()-2);
                }
                if ( argument.second.isnull() )
                    throw exceptions::parse_error( L"Message header " + name + L" does not have a value for an argument", para.first );
                args[ argument.first ] = argument.second.value();
            }
        }
        if ( name == L"Content-Disposition" ) // RFC 1867 uses lower case
            return std::make_pair( L"content-disposition", content( disp.first, args ) );
        else
            return std::make_pair( name, content( disp.first, args ) );
    } else
        return std::make_pair( name, content( value ) );
}


/*
    fostlib::empty_mime
*/


fostlib::empty_mime::empty_mime(
    const mime_headers &headers, const string &mime_type
) : mime( headers, mime_type ) {
    if ( !this->headers().exists(L"Content-Length") )
        this->headers().set("Content-Length", L"0");
}


bool fostlib::empty_mime::boundary_is_ok( const string &/*boundary*/ ) const {
    return true;
}

std::ostream &fostlib::empty_mime::print_on( std::ostream &o ) const {
    return o << headers() << "\r\n";
}


struct fostlib::empty_mime::empty_mime_iterator : public mime::iterator_implementation {
    const_memory_block operator () () {
        return const_memory_block();
    }
};
std::auto_ptr< mime::iterator_implementation > fostlib::empty_mime::iterator() const {
    return std::auto_ptr< mime::iterator_implementation >( new fostlib::empty_mime::empty_mime_iterator );
}


/*
    fostlib::mime_envelope
*/


fostlib::mime_envelope::mime_envelope( const mime_headers &headers,
        const string &type )
: mime( headers, type ) {
}


const fostlib::string &fostlib::mime_envelope::boundary() {
    if ( m_boundary.empty() ) {
        do {
            m_boundary = guid();
        } while ( !boundary_is_ok(m_boundary) );
        mime_headers::content with_boundary(headers()["Content-Type"]);
        with_boundary.subvalue("boundary", boundary());
        headers().set("Content-Type", with_boundary);
    }
    return m_boundary;
}


bool fostlib::mime_envelope::boundary_is_ok( const string &boundary ) const {
    bool ok = true;
    for ( std::list< boost::shared_ptr< mime > >::const_iterator part( items().begin() ); ok && part != items().end(); ++part )
        ok = (*part)->boundary_is_ok(boundary);
    return ok;
}

std::ostream &fostlib::mime_envelope::print_on( std::ostream &o ) const {
    o << headers() << "\r\n";
    for ( const_iterator c(begin()); c != end(); ++c ) {
        const char *first = reinterpret_cast<const char *>((*c).first);
        const char *second = reinterpret_cast<const char *>((*c).second);
        o.write(first, second - first);
    }
    return o;
}


struct fostlib::mime_envelope::mime_envelope_iterator :
        public mime::iterator_implementation {
    enum { e_start, e_attachments, e_final } stage;

    mime_envelope::items_type::const_iterator
        current_attachment, end_attachment;

    boost::scoped_ptr< mime::const_iterator > current, end;

    utf8_string internal_buffer, boundary;

    mime_envelope_iterator(const string &boundary,
        mime_envelope::items_type::const_iterator b,
        mime_envelope::items_type::const_iterator e)
    : stage(e_start), current_attachment(b), end_attachment(e),
            boundary(fostlib::coerce<utf8_string>(boundary)) {
    }

    const_memory_block operator () () {
        static utf8_string dashes("--"), crlf("\r\n");
        switch ( stage ) {
            case e_start:
                internal_buffer = dashes + boundary + crlf;
                stage = e_attachments;
                break;
            case e_attachments:
                if ( !current.get() ) {
                    std::stringstream ss;
                    ss << (*current_attachment)->headers();
                    internal_buffer = utf8_string(ss.str()) + crlf;
                    current.reset(new mime::const_iterator(
                        (*current_attachment)->begin()));
                    end.reset(new mime::const_iterator(
                        (*current_attachment)->end()));
                } else if ( *current != *end ) {
                    const_memory_block r = **current;
                    ++(*current);
                    return r;
                } else {
                    current_attachment++;
                    current.reset(NULL); end.reset(NULL);
                    internal_buffer = crlf + dashes + boundary;
                    if ( current_attachment == end_attachment ) {
                        internal_buffer += dashes + crlf;
                        stage = e_final;
                    } else
                        internal_buffer += crlf;
                }
                break;
            case e_final:
                return const_memory_block();
            default:
                throw exceptions::not_implemented("Impossible MIME envelope write state");
        }
        const std::string &u(internal_buffer.underlying());
        return const_memory_block(u.c_str(), u.c_str() + u.length());
    }
};
std::auto_ptr< mime::iterator_implementation >
        fostlib::mime_envelope::iterator() const {
    return std::auto_ptr< mime::iterator_implementation >(
        new mime_envelope_iterator(m_boundary, items().begin(), items().end()));
}


/*
    fostlib::text_body
*/


namespace {
    void do_headers(text_body &tb, const utf8_string &body, const string &mime_type) {
        if ( !tb.headers().exists("Content-Type") )
            tb.headers().set("Content-Type", mime::mime_headers::content(mime_type) );
        tb.headers().set_subvalue("Content-Type", "charset", "utf-8");
        tb.headers().set(L"Content-Transfer-Encoding", L"8bit");
        tb.headers().set("Content-Length", coerce< string >(body.underlying().length()));
    }
}
fostlib::text_body::text_body(const utf8 *begin, const utf8 *end, const mime_headers &headers, const string &mime_type)
: mime( headers, mime_type ), text(utf8_string(begin, end)) {
    do_headers(*this, text(), mime_type);
}
fostlib::text_body::text_body(const utf8_string &t, const mime_headers &headers, const string &mime_type)
: mime( headers, mime_type ), text(t) {
    do_headers(*this, text(), mime_type);
}
fostlib::text_body::text_body(const fostlib::string &t, const mime_headers &headers, const fostlib::string &mime_type)
: mime( headers, mime_type ), text(coerce< utf8_string >(t)) {
    do_headers(*this, text(), mime_type);
}

std::ostream &fostlib::text_body::print_on( std::ostream &o ) const {
    return o << headers() << "\r\n" << text().underlying();
}

bool fostlib::text_body::boundary_is_ok( const string &boundary ) const {
    return text().underlying().find( coerce< utf8_string >( boundary ).underlying() ) == string::npos;
}


struct fostlib::text_body::text_body_iterator : public mime::iterator_implementation {
    const utf8_string &body; bool sent;
    text_body_iterator(const utf8_string &b)
    : body(b), sent(false) {
    }
    const_memory_block operator () () {
        if ( !body.underlying().length() || sent )
            return const_memory_block();
        else {
            sent = true;
            return const_memory_block( body.underlying().c_str(), body.underlying().c_str() + body.underlying().length() );
        }
    }
};
std::auto_ptr< mime::iterator_implementation > fostlib::text_body::iterator() const {
    return std::auto_ptr< mime::iterator_implementation >(new text_body_iterator(text()));
}


/*
    fostlib::binary_body
*/


fostlib::binary_body::binary_body(
    const mime_headers &headers,
    const string &mime_type
) : mime(headers, mime_type) {
}
fostlib::binary_body::binary_body(
    const std::vector< unsigned char > &data,
    const mime_headers &headers,
    const string &mime_type
) : mime(headers, mime_type), data(data) {
}

std::ostream &fostlib::binary_body::print_on( std::ostream &o ) const {
    for (
        std::vector< unsigned char >::const_iterator i(data().begin());
        i != data().end();
        ++i
    )
        o << int(*i) << ' ';
    return o;
}

bool fostlib::binary_body::boundary_is_ok( const string &boundary ) const {
    throw exceptions::not_implemented(
        "fostlib::binary_body::boundary_is_ok( const string &boundary ) const"
    );
}

struct fostlib::binary_body::binary_body_iterator :
    public mime::iterator_implementation
{
    const std::vector< unsigned char > &data; bool sent;
    binary_body_iterator(const std::vector< unsigned char > &d)
    : data(d), sent(false) {
    }
    const_memory_block operator () () {
        if ( data.size() == 0 || sent )
            return const_memory_block();
        else {
            sent = true;
            return const_memory_block(&data[0], &data[0] + data.size());
        }
    }
};
std::auto_ptr< mime::iterator_implementation > fostlib::binary_body::iterator() const {
    return std::auto_ptr< mime::iterator_implementation >(
        new binary_body_iterator(data())
    );
}


/*
    fostlib::file_body
*/


fostlib::file_body::file_body( const boost::filesystem::wpath &p, const mime_headers &headers, const string &mime_type )
: mime( headers, mime_type ), filename( p ) {
    this->headers().set(L"Content-Transfer-Encoding", L"8bit");
    this->headers().set(L"Content-Length", coerce< string >( boost::filesystem::file_size(p) ));
}


std::ostream &fostlib::file_body::print_on( std::ostream &o ) const {
    boost::filesystem::ifstream file(filename(), std::ios::binary);
    return o << headers() << "\r\n" << file.rdbuf();
}

bool fostlib::file_body::boundary_is_ok( const string &boundary ) const {
    throw exceptions::not_implemented(
        "fostlib::file_body::boundary_is_ok( const string &boundary ) const"
    );
}


struct fostlib::file_body::file_body_iteration : public mime::iterator_implementation {
    boost::filesystem::ifstream file;
    boost::array< char, 2048 > buffer;
    file_body_iteration( const boost::filesystem::wpath &p )
    : file( p, std::ios::binary ) {
    }
    const_memory_block operator () () {
        if ( !file.eof() && file.good() ) {
            file.read(buffer.c_array(), buffer.size());
            return const_memory_block( buffer.data(), buffer.data() + file.gcount() );
        } else
            return const_memory_block();
    }
};
std::auto_ptr< mime::iterator_implementation > fostlib::file_body::iterator() const {
    return std::auto_ptr< mime::iterator_implementation >(new file_body_iteration(filename()));;
}

