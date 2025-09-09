local ls = require("luasnip")
local s = ls.snippet
local t = ls.text_node
local i = ls.insert_node
local f = ls.function_node
local d = ls.dynamic_node
local sn = ls.snippet_node

return {
  s("cmakemin", {
    t('cmake_minimum_required(VERSION '), i(1, "3.10"), t(')'),
    t({"", ""}), i(0)
  }),

  s("cmakeproj", {
    t('project('), i(1, "ProjectName"),
    t({" VERSION ", ""}), i(2, "1.0"),
    t({" LANGUAGES ", ""}), i(3, "CXX"),
    t(')'),
    t({"", ""}), i(0)
  }),

  s("cmakecxx", {
    t('set(CMAKE_CXX_STANDARD '), i(1, "17"), t(')'),
    t({"", 'set(CMAKE_CXX_STANDARD_REQUIRED ON)'}),
    t({"", 'set(CMAKE_CXX_EXTENSIONS OFF)'}),
    t({"", ""}), i(0)
  }),

  s("cmakec", {
    t('set(CMAKE_C_STANDARD '), i(1, "99"), t(')'),
    t({"", 'set(CMAKE_C_STANDARD_REQUIRED ON)'}),
    t({"", 'set(CMAKE_C_EXTENSIONS OFF)'}),
    t({"", ""}), i(0)
  }),

  s("cmakesrc", {
    t('add_subdirectory('), i(1, "src"), t(')'),
    t({"", ""}), i(0)
  }),

  s("cmaketest", {
    t({'enable_testing()', ''}),
    t({'', 'add_subdirectory('}), i(1, "tests"), t(')'),
    t({"", ""}), i(0)
  }),

  s("cmakeexe", {
    t('add_executable('), i(1, "${PROJECT_NAME}"), t(' '),
    i(2, "main.cpp"),
    t(')'),
    t({"", ""}), i(0)
  }),

  s("cmakelib", {
    t('add_library('), i(1, "mylibrary"),
    t({" ", ""}), i(2, "STATIC"),
    t({" ", ""}), i(3, "src/library.cpp"),
    t(')'),
    t({"", ""}), i(0)
  }),

  s("cmakefind", {
    t('find_package('), i(1, "Boost"), t(' REQUIRED COMPONENTS '),
    i(2, "system filesystem"),
    t(')'),
    t({"", ""}), i(0)
  }),

  s("cmakelink", {
    t('target_link_libraries('), i(1, "${PROJECT_NAME}"), t(' PRIVATE '),
    i(2, "mylibrary"),
    t(')'),
    t({"", ""}), i(0)
  }),

  s("cmakeinc", {
    t('target_include_directories('), i(1, "${PROJECT_NAME}"), t(' PRIVATE '),
    i(2, "${CMAKE_CURRENT_SOURCE_DIR}/include"),
    t(')'),
    t({"", ""}), i(0)
  }),

  s("cmakecompile", {
    t('target_compile_options('), i(1, "${PROJECT_NAME}"), t(' PRIVATE'),
    t({"", '\t$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>'}),
    t({"", '\t$<$<CXX_COMPILER_ID:MSVC>:/W4>'}),
    t(')'),
    t({"", ""}), i(0)
  })
}
