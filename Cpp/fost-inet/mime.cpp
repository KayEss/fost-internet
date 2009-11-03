/*
    Copyright 1999-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include "fost-inet.hpp"
#include <fost/detail/mime.hpp>
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
        current = std::pair< const unsigned char *, const unsigned char * >( NULL, NULL );
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


fostlib::empty_mime::empty_mime( const mime_headers &headers )
: mime( headers, "application/x-empty" ) {
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
        return const_memory_block( NULL, NULL );
    }
};
std::auto_ptr< mime::iterator_implementation > fostlib::empty_mime::iterator() const {
    return std::auto_ptr< mime::iterator_implementation >( new fostlib::empty_mime::empty_mime_iterator );
}


/*
    fostlib::mime_envelope
*/


fostlib::mime_envelope::mime_envelope( const mime_headers &headers )
: mime( headers, "multipart/mixed" ) {
}


bool fostlib::mime_envelope::boundary_is_ok( const string &boundary ) const {
    bool ok = true;
    for ( std::list< boost::shared_ptr< mime > >::const_iterator part( items().begin() ); ok && part != items().end(); ++part )
        ok = (*part)->boundary_is_ok(boundary);
    return ok;
}

std::ostream &fostlib::mime_envelope::print_on( std::ostream &o ) const {
    string boundary;
    do {
        boundary = guid();
    } while ( !boundary_is_ok( boundary ) );

    mime_headers local_headers = headers();
    mime_headers::content with_boundary(headers()["Content-Type"]);
    with_boundary.subvalue("boundary", boundary);
    local_headers.set("Content-Type", with_boundary);

    o << local_headers;
    for ( std::list< boost::shared_ptr< mime > >::const_iterator part( items().begin() ) ;part != items().end(); ++part ) {
        o << "\r\n--" << coerce< utf8string >( boundary ) << "\r\n";
        (*part)->print_on( o );
    }
    return o << "\r\n--" << coerce< utf8string >( boundary ) << "--\r\n";
}


namespace {
}
std::auto_ptr< mime::iterator_implementation > fostlib::mime_envelope::iterator() const {
    throw exceptions::not_implemented("fostlib::mime_envelope::iterator() const");
}


/*
    fostlib::text_body
*/


namespace {
    void do_headers(text_body &tb, const utf8string &body, const string &mime_type) {
        if ( !tb.headers().exists("Content-Type") )
            tb.headers().set("Content-Type", mime::mime_headers::content(mime_type) );
        tb.headers().set_subvalue("Content-Type", "charset", "utf-8");
        tb.headers().set(L"Content-Transfer-Encoding", L"8bit");
        tb.headers().set("Content-Length", coerce< string >(body.length()));
    }
}
fostlib::text_body::text_body(const utf8 *begin, const utf8 *end, const mime_headers &headers, const string &mime_type)
: mime( headers, mime_type ), text(utf8string(begin, end)) {
    do_headers(*this, text(), mime_type);
}
fostlib::text_body::text_body(const utf8string &t, const mime_headers &headers, const string &mime_type)
: mime( headers, mime_type ), text(t) {
    do_headers(*this, text(), mime_type);
}
fostlib::text_body::text_body(const fostlib::string &t, const mime_headers &headers, const fostlib::string &mime_type)
: mime( headers, mime_type ), text(coerce< utf8string >(t)) {
    do_headers(*this, text(), mime_type);
}

std::ostream &fostlib::text_body::print_on( std::ostream &o ) const {
    return o << headers() << "\r\n" << text();
}

bool fostlib::text_body::boundary_is_ok( const string &boundary ) const {
    return text().find( coerce< utf8string >( boundary ) ) == string::npos;
}


struct fostlib::text_body::text_body_iterator : public mime::iterator_implementation {
    const utf8string &body; bool sent;
    text_body_iterator(const utf8string &b)
    : body(b), sent(false) {
    }
    const_memory_block operator () () {
        if ( !body.length() || sent )
            return const_memory_block( NULL, NULL );
        else {
            sent = true;
            return const_memory_block( body.c_str(), body.c_str() + body.length() );
        }
    }
};
std::auto_ptr< mime::iterator_implementation > fostlib::text_body::iterator() const {
    return std::auto_ptr< mime::iterator_implementation >(new text_body_iterator(text()));
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
    throw exceptions::not_implemented("fostlib::file_body::boundary_is_ok( const string &boundary ) const");
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
            return const_memory_block( NULL, NULL );
    }
};
std::auto_ptr< mime::iterator_implementation > fostlib::file_body::iterator() const {
    return std::auto_ptr< mime::iterator_implementation >(new file_body_iteration(filename()));;
}

