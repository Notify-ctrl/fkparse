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

include_directories(src)
include_directories(src/analyze)
include_directories(src/parse)
include_directories(src/fundamental)

add_subdirectory(src)
