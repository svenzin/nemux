file(GLOB SOURCES "*.cpp" "*.h")
add_executable(nemux-vis ${SOURCES})

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(nemux-vis nemux SDL2::SDL2 SDL2::SDL2main)
