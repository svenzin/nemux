set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

file (GLOB SOURCES "*.cpp")

include_directories (${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories (${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
include_directories (../nemux)

add_executable (nemux-test ${SOURCES})
target_link_libraries (nemux-test nemux gmock_main)