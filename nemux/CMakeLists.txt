# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

file(GLOB SOURCES "*.cpp" "*.h")

add_library(nemux ${SOURCES})
target_include_directories(nemux PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
