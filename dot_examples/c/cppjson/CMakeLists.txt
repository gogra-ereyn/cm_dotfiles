cmake_minimum_required(VERSION 3.15)
project(main C CXX)

include(FetchContent)

FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG        v4.0.5
  GIT_SHALLOW    TRUE
)

set(SIMDJSON_JUST_LIBRARY ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(simdjson)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FAST_OPTS "-march=native -pipe -flto")
set(CMAKE_C_FLAGS_RELEASE          "${FAST_OPTS} -O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "${FAST_OPTS} -O2 -g")
set(CMAKE_C_FLAGS_DEBUG            "-O0 -g3 -ggdb")

set(CMAKE_CXX_FLAGS_RELEASE        "${FAST_OPTS} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${FAST_OPTS} -O2 -g")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g3 -ggdb")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -msse4.2 -mavx2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mavx2")
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--copy-dt-needed-entries")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--copy-dt-needed-entries")
set(CMAKE_STATIC_LINKER_FLAGS "-Wl,--copy-dt-needed-entries")

file(GLOB SRCS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)
add_executable(main ${SRCS})
target_link_libraries(main PRIVATE simdjson protobuf::libprotobuf absl::log_internal_check_op)
