cmake_minimum_required(VERSION 3.16)

project(fkparse VERSION 0.0.1)

include_directories(src)
include_directories(src/analyze)
include_directories(src/parse)
include_directories(src/fundamental)

add_subdirectory(src)
