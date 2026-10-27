cmake_minimum_required(VERSION 3.15)
project(cutestrs C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)
else()
    include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
endif()

add_library(strlib OBJECT cute_str.c)

add_executable(qw qw.c)
target_link_libraries(qw strlib)

if(BUILD_TESTING)
    add_executable(cutestr_tests tests/test_qw.c)
    target_link_libraries(cutestr_tests strlib)
    add_test(NAME cutestr_tests COMMAND cutestr_tests)
    add_test(NAME cutestr_tests_e2e COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_qw.sh $<TARGET_FILE:qw>)
endif()

install(TARGETS qw)
