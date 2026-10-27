cmake_minimum_required(VERSION 3.15)
project(cpy VERSION 0.1.1 LANGUAGES C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)
else()
    include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)
endif()

add_executable(cpy cpy.c)
target_compile_definitions(cpy PRIVATE CPY_VERSION="${PROJECT_VERSION}")

if(BUILD_TESTING)
    add_test(NAME cpy_tests
             COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.sh $<TARGET_FILE:cpy>)
endif()

install(TARGETS cpy)
