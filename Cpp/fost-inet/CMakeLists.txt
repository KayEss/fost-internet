add_library(fost-inet
        connection.cpp
        email.cpp
        fost-inet.cpp
        headers.cpp
        host.cpp
        http.authentication.cpp
        http.cache.cpp
        http.server.cpp
        http.useragent.cpp
        mime.cpp
        pop3.cpp
        smtp.cpp
        url-query_string.cpp
        url.cpp
    )
target_link_libraries(fost-inet fost-crypto crypto ssl)
target_include_directories(fost-inet PUBLIC ../include)
set_target_properties(fost-inet PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS fost-inet LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

if(TARGET check)
    add_library(fost-inet-smoke STATIC EXCLUDE_FROM_ALL
            authentication-tests.cpp
            headers.cookie.tests.cpp
            headers-tests.cpp
            host-tests.cpp
            http-tests.cpp
            http.cache.tests.cpp
            mime-binary_body-tests.cpp
            mime-mime_envelope-tests.cpp
            mime-tests.cpp
            smtp-tests.cpp
            url-tests.cpp
        )
    target_link_libraries(fost-inet-smoke fost-inet)
    smoke_test(fost-inet-smoke)
endif()
