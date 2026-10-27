cmake_minimum_required(VERSION 3.15)
project(fmb C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)
else()
    include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
endif()

add_executable(fmb ${CMAKE_CURRENT_SOURCE_DIR}/fmb.c)

if(BUILD_TESTING)
    add_test(
        NAME fmb_tests
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.sh $<TARGET_FILE:fmb>)
endif()

install(TARGETS fmb)
