cmake_minimum_required(VERSION 3.15)
project(cutenums C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)
else()
    include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
endif()

add_executable(p2above p2above.c)
add_executable(p2below p2below.c)

if(BUILD_TESTING)
    add_test(
        NAME cutenums_tests
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.sh $<TARGET_FILE:p2above> $<TARGET_FILE:p2below>)
endif()

install(TARGETS p2above p2below)
