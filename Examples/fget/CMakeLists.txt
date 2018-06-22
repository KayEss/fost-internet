add_executable(fget EXCLUDE_FROM_ALL
        fget.cpp
    )
target_link_libraries(fget fost-cli fost-inet)
add_dependencies(stress fget)

add_custom_command(TARGET fget
    POST_BUILD COMMAND fget https://kirit.com/)
add_test(NAME fget-kirit COMMAND fget https://kirit.com/)

add_custom_command(TARGET fget
    POST_BUILD COMMAND fget http://static.felspar.com/felspar-180x53.png/)
add_test(NAME fget-image COMMAND fget http://static.felspar.com/felspar-180x53.png)
