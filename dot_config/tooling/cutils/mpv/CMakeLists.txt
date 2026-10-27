cmake_minimum_required(VERSION 3.15)
project(mvp C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)
else()
    include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
endif()

file(GLOB MVF_SRCS CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
add_executable(mvp ${MVF_SRCS})

if(BUILD_TESTING)
    add_test(
        NAME mpv_basic_literal
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_literal.sh $<TARGET_FILE:mvp>)
endif()

install(TARGETS mvp)
