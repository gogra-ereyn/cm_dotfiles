cmake_minimum_required(VERSION 3.15)
project(cuteid C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)
else()
    include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
endif()

add_executable(cuteid cuteid.c)

if(BUILD_TESTING)
    add_test(NAME test_cuteid COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cuteid.sh $<TARGET_FILE:cuteid>)
endif()

install(TARGETS cuteid)
