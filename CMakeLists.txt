cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/vcpkg/scripts/buildsystems/vcpkg.cmake
    # CACHE STRING "Vcpkg toolchain file"
)

project(nemux)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set_property (GLOBAL PROPERTY USE_FOLDERS ON)

# if (MSYS)
#     message (STATUS "Disable pthreads for gtest")
#     set (gtest_disable_pthreads ON CACHE BOOL "Disable pthreads for MSYS" FORCE)
# endif (MSYS)

# message ("cxx flags: " ${CMAKE_CXX_FLAGS})

add_subdirectory(nemux)

add_subdirectory(nemux-cli)

add_subdirectory(nemux-vis)

add_subdirectory(nemux-test)
