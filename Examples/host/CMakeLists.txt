add_executable(host EXCLUDE_FROM_ALL
        host-main.cpp
    )
target_link_libraries(host fost-cli fost-inet)
add_dependencies(examples host)

