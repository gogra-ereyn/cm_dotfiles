cmake_minimum_required(VERSION 3.14)
project(cb_example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(cb_test test.cpp)
target_link_libraries(cb_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(cb_test)
