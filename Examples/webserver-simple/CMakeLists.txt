add_executable(http-simple simple.cpp)
target_link_libraries(http-simple fost-cli fost-inet)
install(TARGETS http-simple EXPORT http-simple RUNTIME DESTINATION bin)

