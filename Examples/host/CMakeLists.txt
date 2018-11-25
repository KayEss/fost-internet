add_executable(host host-main.cpp)
target_link_libraries(host fost-cli fost-inet)
install(TARGETS host EXPORT host RUNTIME DESTINATION bin)

