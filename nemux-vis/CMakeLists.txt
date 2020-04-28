set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

file (GLOB SOURCES "*.cpp" "*.h")

include_directories (../3rd-party/SDL2-2.0.9/include)
link_directories (../3rd-party/SDL2-2.0.9/lib/x86)

file(COPY ../3rd-party/SDL2-2.0.9/lib/x86/SDL2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories (../nemux)

add_executable (nemux-vis ${SOURCES})
target_link_libraries (nemux-vis nemux SDL2 SDL2main)
