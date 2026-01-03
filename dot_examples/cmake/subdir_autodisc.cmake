
file(GLOB children RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/*)

foreach(child ${children})
    if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${child}
       AND EXISTS ${CMAKE_SOURCE_DIR}/${child}/CMakeLists.txt
       AND NOT child STREQUAL "cmake")
        message(STATUS "adding tool subdir: ${child}")
        add_subdirectory(${child})
    endif()
endforeach()

