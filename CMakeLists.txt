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

add_custom_command(
  PRE_BUILD
  OUTPUT ${PROJECT_SOURCE_DIR}/src/parse/grammar.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/parse/grammar.y
  COMMENT "Generating grammar.c"
  COMMAND ${BISON} -d ${PROJECT_SOURCE_DIR}/src/parse/grammar.y -o ${PROJECT_SOURCE_DIR}/src/parse/grammar.c
)

add_custom_command(
  PRE_BUILD
  OUTPUT ${PROJECT_SOURCE_DIR}/src/parse/lex.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/parse/lex.l ${PROJECT_SOURCE_DIR}/src/parse/grammar.c
  COMMENT "Generating lex.c"
  COMMAND ${FLEX} -o ${PROJECT_SOURCE_DIR}/src/parse/lex.c ${PROJECT_SOURCE_DIR}/src/parse/lex.l
)

if (${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  add_executable(fkparse)
else ()
  add_library(fkparse SHARED)
endif ()
add_subdirectory(src)

#find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)
#set(CMAKE_AUTOMOC ON)
#qt_add_executable(fkparse_qt)
#add_subdirectory(gui)
#target_link_libraries(fkparse_qt PRIVATE fkparse Qt6::Gui Qt6::Widgets)

