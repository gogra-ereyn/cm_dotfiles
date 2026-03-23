cmake_minimum_required(VERSION 3.15)
project(main)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -march=native -pipe")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -ggdb")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.2 -mavx2")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_default_testing ON)
else()
    set(_default_testing OFF)
endif()
option(BUILD_TESTING "Build tests" ${_default_testing})

add_executable(main ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    if(BUILD_TESTING)
        include(CTest)
        enable_testing()
        add_executable(tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.c)
        add_test(
            NAME run_tests COMMAND tests
        )
    endif()
endif()
