add_executable(fget fget.cpp)
target_link_libraries(fget fost-cli fost-inet)
install(TARGETS fget EXPORT fget RUNTIME DESTINATION bin)

