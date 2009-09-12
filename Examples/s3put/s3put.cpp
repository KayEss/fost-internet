/*
    Copyright 2008-2009, Felspar Co Ltd. http://fost.3.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/cli>
#include <fost/main>
#include <fost/unicode>
#include <fost/aws>


using namespace fostlib;


FSL_MAIN(
    L"s3put",
    L"Amazon S3 client -- push to S3\nCopyright (C) 2008-2009, Felspar Co. Ltd."
)( fostlib::ostream &o, fostlib::arguments &args ) {
    // Check we have the minimum number of command line arguments
    if ( args.size() < 4 ) {
        o << "Must supply at least one file and a bucket name and an S3 location to put it to" << std::endl;
        return 1;
    }
    // Create the bucket object
    aws::s3::bucket bucket(coerce< ascii_string >( args[2] ));
    // Local file information
    digester md5_digest(md5);
    md5_digest << coerce< boost::filesystem::wpath >( args[1].value() );
    string local_md5 = coerce< string >( coerce< hex_string >( md5_digest.digest() ) );
    o << "Local file " << args[1].value() << " md5: " << local_md5 << std::endl;
    // Remote file information
    aws::s3::file_info remote(bucket.stat(coerce< boost::filesystem::wpath >( args[3].value() )));
    o << "Remote file MD5 " << remote.md5().value(L"[No remote file MD5]") << std::endl;
    // Decide if we're going to upload the file or not
    if ( !remote.exists() || remote.md5() != L"\"" + local_md5 + L"\"" ) {
        o << L"Uploading file..." << std::endl;
        bucket.put(coerce< boost::filesystem::wpath >( args[1].value() ), remote.path());
        o << L"Upload complete" << std::endl;
    } else
        o << L"The remote file already exists and is identical to the local file. No file has been uploaded" << std::endl;
    return 0;
}
