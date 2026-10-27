# Include guard
if(_CUTILS_COMMON_INCLUDED)
    return()
endif()
set(_CUTILS_COMMON_INCLUDED TRUE)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_compile_definitions(_POSIX_C_SOURCE=200809L)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # static libs
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # shared libs

set(FAST_OPTS "-march=native -pipe -flto")

set(CMAKE_C_FLAGS_RELEASE        "${FAST_OPTS} -O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${FAST_OPTS} -O2 -g")
set(CMAKE_C_FLAGS_DEBUG          "-O0 -g3 -ggdb")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install prefix" FORCE)
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.2 -mavx2")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Shared include directory (include/cute_*.h)
get_filename_component(_CUTILS_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
include_directories(${_CUTILS_ROOT}/include)

option(BUILD_TESTING "Build tests" ON)
include(CTest)

function(add_tool_tests TOOL_NAME)
    if(NOT BUILD_TESTING)
        return()
    endif()
    file(GLOB TEST_SRCS CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c")
    if(TEST_SRCS)
        add_executable(${TOOL_NAME}_tests ${TEST_SRCS})
        add_test(NAME ${TOOL_NAME}_tests COMMAND ${TOOL_NAME}_tests)
    endif()
endfunction()
