add_executable(http-threaded threaded.cpp)
target_link_libraries(http-threaded fost-cli fost-inet)
install(TARGETS http-threaded EXPORT http-threaded RUNTIME DESTINATION bin)

