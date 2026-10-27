cmake_minimum_required(VERSION 3.15)
project(surround C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)
else()
    include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
endif()

add_executable(surround surround.c)

if(BUILD_TESTING)
    add_test(
        NAME surround_tests
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.sh $<TARGET_FILE:surround>)
endif()

install(TARGETS surround)
