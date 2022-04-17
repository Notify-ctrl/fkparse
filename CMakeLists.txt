cmake_minimum_required(VERSION 3.16)

project(fkparse VERSION 0.0.1)

#find_package(Qt5 REQUIRED COMPONENTS
#    Core
#)

#set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(REQUIRED_QT_VERSION "5.15.2")

include_directories(src)

add_subdirectory(src)

