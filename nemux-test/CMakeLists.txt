add_subdirectory(cpu)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(nemux-test ${SOURCES})

enable_testing()

find_package(GTest CONFIG REQUIRED)
target_link_libraries(nemux-test nemux GTest::gtest GTest::gmock GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(nemux-test)
# gtest_add_tests(TARGET nemux-test)
# add_test(TARGET nemux-test)
