add_library(fost-inet-test STATIC EXCLUDE_FROM_ALL
        configuration.cpp
        connection-failures.cpp
        data-transmission.cpp
        host.cpp
        pop3.cpp
        smtp.cpp
        user_agent.cpp
    )
target_link_libraries(fost-inet-test fost-inet)
stress_test(fost-inet-test)
