if(TARGET stress)
    # Split the tests into parts -- those that require external configuration
    # before they can be run, and those that do not. The simpler libraries
    # come first.
    add_library(fost-inet-test STATIC EXCLUDE_FROM_ALL
            connection-failures.cpp
            data-transmission.cpp
            host.cpp
            user_agent.cpp
        )
    target_link_libraries(fost-inet-test fost-inet)
    stress_test(fost-inet-test)

    # These are the tests that require the external configuration
    add_library(fost-inet-test-with-config STATIC EXCLUDE_FROM_ALL
            configuration.cpp
            pop3.cpp
            smtp.cpp
            tls.cpp
        )
    target_link_libraries(fost-inet-test-with-config fost-inet)
    stress_test(fost-inet-test-with-config)
endif()
