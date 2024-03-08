if(TARGET stress)
    # Split the tests into parts -- those that require external configuration
    # before they can be run, and those that do not. The simpler libraries
    # come first.
    add_test_run(stress fost-inet TESTS
            cache.cpp
            connection-failures.cpp
            data-transmission.cpp
            host.cpp
            http.cpp
            user_agent.cpp
        )

    # These are the tests that require the external configuration
    add_library(fost-inet-test-configuration
            configuration.cpp
        )
    target_link_libraries(fost-inet-test-configuration PUBLIC fost-inet)

    add_test_run(stress
            fost-core-test
            fost-inet
            fost-inet-test-configuration
            TESTS
            pop3.cpp
            smtp.cpp
            tls.cpp
        )
endif()
