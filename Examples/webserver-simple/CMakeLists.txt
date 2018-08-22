add_executable(http-simple EXCLUDE_FROM_ALL
        simple.cpp
    )
target_link_libraries(http-simple fost-cli fost-inet)
add_dependencies(examples http-simple)

