file(GLOB SOURCES "*.cpp")
add_executable(nemux-test ${SOURCES})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(nemux-test nemux GTest::gtest GTest::gmock GTest::gmock_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(nemux-test)
# gtest_add_tests(TARGET nemux-test)
# add_test(TARGET nemux-test)
