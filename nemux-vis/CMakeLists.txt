# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

file(GLOB SOURCES "*.cpp" "*.h")

# include_directories(${CMAKE_SOURCE_DIR}/3rd-party/SDL2-2.0.9/include)
# link_directories(${CMAKE_SOURCE_DIR}/3rd-party/SDL2-2.0.9/lib/x64)

# file(COPY ${CMAKE_SOURCE_DIR}/3rd-party/SDL2-2.0.9/lib/x64/SDL2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# include_directories (../nemux)

add_executable(nemux-vis ${SOURCES})
target_link_libraries(nemux-vis nemux SDL2::SDL2 SDL2::SDL2main)

# add_custom_command(
#     TARGET nemux-vis POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/3rd-party/SDL2-2.0.9/lib/x64/SDL2.dll" $<TARGET_FILE_DIR:nemux-vis>
#     COMMENT "Copying SDL2.dll" VERBATIM
# )
