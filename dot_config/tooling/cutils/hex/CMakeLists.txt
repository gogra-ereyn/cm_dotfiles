cmake_minimum_required(VERSION 3.15)
project(hex C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)
else()
    include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
endif()

add_executable(hex hex.c)
add_executable(itoh itoh.c)
add_executable(htoi htoi.c)

if(BUILD_TESTING)
    add_test(NAME test_hex COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_hex.sh $<TARGET_FILE:hex>)
endif()

install(TARGETS itoh htoi hex)
