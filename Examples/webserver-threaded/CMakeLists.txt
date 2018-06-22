add_executable(http-threaded EXCLUDE_FROM_ALL
        threaded.cpp
    )
target_link_libraries(http-threaded fost-cli fost-inet)
add_dependencies(examples http-threaded)

