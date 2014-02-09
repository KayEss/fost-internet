/** \defgroup fost_inet_mime MIME
    \ingroup fost_inet

    \brief MIME containers and MIME headers..

    MIME is a key abstraction for a number of Internet protocol and it is also used as an abstraction to decouple parts of the Fost libraries.
*/


/** \class fostlib::mime fost/detail/mime.hpp fost/internet
    \ingroup fost_inet_mime

    This initial implementation is only intended to be able to parse HTTP POST multipart/form-data encoded data and HTTP host headers. A later implementation will need to consider the transport parameters in order to know how to encode the content and which headers are required.

    This superclass allows MIME headers to be queried and set and also allows for the MIME contents to be traversed.
*/
/** \class fostlib::mime::mime_headers fost/detail/mime.hpp fost/internet
    \ingroup fost_inet_mime
*/


/** \class fostlib::headers_base fost/detail/mime.hpp fost/internet
    \ingroup fost_inet_mime

    MIME headers may have multiple entries for a given header name. For example, mail headers will contain multiple "Received" headers, one for each mail server the email has been processed by.

    The <code>set</code> members will delete any old headers with the same value. The <code>add</code> members will add additional headers of a given name.
*/


/** \class fostlib::empty_mime fost/detail/mime.hpp fost/internet
    \ingroup fost_inet_mime
*/


/** \class fostlib::mime_envelope fost/detail/mime.hpp fost/internet
    \ingroup fost_inet_mime
*/


/** \class fostlib::text_body fost/detail/mime.hpp fost/internet
    \ingroup fost_inet_mime
*/


/** \class fostlib::file_body fost/detail/mime.hpp fost/internet
    \ingroup fost_inet_mime
*/
