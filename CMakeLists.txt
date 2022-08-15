cmake_minimum_required(VERSION 3.16)

project(fkparse VERSION 0.0.1)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions(-DFK_DEBUG)
endif ()

if (WIN32)
  add_definitions(-DFK_WIN32)
  set(FLEX win_flex)
  set(BISON win_bison)
else ()
  set(FLEX flex)
  set(BISON bison)
endif ()

add_library(fkparse SHARED)
add_subdirectory(src)
