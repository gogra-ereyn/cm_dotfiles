cmake_minimum_required(VERSION 3.15)
project(ctee)
add_executable(ctee src/main.c)
add_custom_command(TARGET ctee POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:ctee>
        ${CMAKE_SOURCE_DIR}/run
    COMMENT "Creating symlink to ctee in project root"
)

