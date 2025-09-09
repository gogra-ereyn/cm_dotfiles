include(ExternalProject)
ExternalProject_Add(liburing
  GIT_REPOSITORY    https://github.com/axboe/liburing.git
  GIT_TAG           master
  PREFIX            ${CMAKE_CURRENT_BINARY_DIR}/liburing
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
  BUILD_IN_SOURCE   1
)

add_library(uring STATIC IMPORTED)
set_target_properties(uring PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/liburing/lib/liburing.a
)
add_dependencies(uring liburing)
target_include_directories(my_program PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/liburing/include)
target_link_libraries(my_program PRIVATE uring pthread)
