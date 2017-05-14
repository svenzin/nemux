cmake_minimum_required (VERSION 2.6)

project (nemux)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

message ("cxx flags: " ${CMAKE_CXX_FLAGS})

add_subdirectory (nemux)
add_subdirectory (3rd-party/googletest)
add_subdirectory (nemux-test)

enable_testing ()
