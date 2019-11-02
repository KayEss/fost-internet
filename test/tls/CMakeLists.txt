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

    ssltest(sha256.badssl.com true)
    ssltest(sha384.badssl.com true)
    ssltest(sha512.badssl.com true)

    ssltest(not-valid.example.com false)

    add_custom_target(fost-internet-ssl-tests
        DEPENDS ${fost-internet-ssl-test-list})
    set_property(TARGET fost-internet-ssl-tests  PROPERTY EXCLUDE_FROM_ALL TRUE)
    add_dependencies(stress fost-internet-ssl-tests)
endif()
