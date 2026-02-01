cmake_minimum_required(VERSION 3.15)
project(main)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -march=native -pipe")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -ggdb")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.2 -mavx2")
endif()

add_executable(main ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BUILD_TESTING "build tests" ON)
    if(BUILD_TESTING)
        enable_testing()
        add_executable(tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.c)
        add_test(
            NAME run_tests COMMAND tests
        )
    endif()
endif()
