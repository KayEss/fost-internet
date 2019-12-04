project(fost-internet)

if(TARGET stress)
    set_property(TARGET stress PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()

add_subdirectory(Configuration)
add_subdirectory(Cpp)
add_subdirectory(test)
