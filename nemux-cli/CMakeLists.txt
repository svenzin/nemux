set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

file (GLOB SOURCES "*.cpp" "*.h")

include_directories (../nemux)

add_executable (nemux-cli ${SOURCES})
target_link_libraries (nemux)
