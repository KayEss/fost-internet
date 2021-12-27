if(TARGET stress)
    add_executable(test-ssl EXCLUDE_FROM_ALL test-ssl.cpp)
    target_link_libraries(test-ssl fost-cli fost-inet)

    set(fost-internet-ssl-test-list)
    macro(ssltest host pass)
        add_custom_command(OUTPUT fost-internet-ssl-test-${host}
            COMMAND test-ssl -b false -w ${pass}
                "https://${host}/" -o fost-internet-ssl-test-${host}
            DEPENDS test-ssl)
        list(APPEND fost-internet-ssl-test-list fost-internet-ssl-test-${host})
    endmacro()

    ## These tests are executed against https://badssl.com/ which contains
    ## a large number of TLS/SSL test cases. Only the most important are
    ## currently checked.
    ssltest(sha256.badssl.com true)
    ssltest(sha384.badssl.com true)
    ssltest(sha512.badssl.com true)

    ssltest(not-valid.example.com false)
    ssltest(expired.badssl.com false)
    ssltest(wrong.host.badssl.com false)
    ssltest(self-signed.badssl.com false)
    ssltest(untrusted-root.badssl.com false)

    ## The following return the wrong results. These tests are just to mark
    ## the current behaviour and to notify any changes due to changes in
    ## the underlying Boost/ASIO/OpenSSL libraries being used.
    ssltest(revoked.badssl.com true)
    ssltest(pinning-test.badssl.com true)
    ssltest(1000-sans.badssl.com false)
    ssltest(10000-sans.badssl.com false)

    add_custom_target(fost-internet-ssl-tests
        DEPENDS ${fost-internet-ssl-test-list})
    set_property(TARGET fost-internet-ssl-tests  PROPERTY EXCLUDE_FROM_ALL TRUE)
    add_dependencies(stress fost-internet-ssl-tests)
endif()
