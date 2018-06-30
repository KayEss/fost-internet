add_library(fost-hod
        connection.cpp
        counters.cpp
        hod.cpp
        packet.cpp
    )
target_link_libraries(fost-hod fost-crypto fost-inet)
target_compile_definitions(fost-hod
        PUBLIC BOOST_COROUTINES_NO_DEPRECATION_WARNING
        PUBLIC BOOST_COROUTINE_NO_DEPRECATION_WARNING
    )
install(TARGETS fost-hod LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
