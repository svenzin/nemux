file(GLOB SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(nemux-test-cpu ${SOURCES})

enable_testing()

find_package(GTest CONFIG REQUIRED)
target_link_libraries(nemux-test-cpu nemux GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(nemux-test-cpu)
# gtest_add_tests(TARGET nemux-test-cpu)
# add_test(TARGET nemux-test-cpu)
