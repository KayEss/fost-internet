add_executable(fget EXCLUDE_FROM_ALL
        fget.cpp
    )
target_link_libraries(fget fost-cli fost-inet)
add_dependencies(examples hash)
