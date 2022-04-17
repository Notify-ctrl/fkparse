set(fkparse_SRCS
  "main.c"
  "lex.c"
  "grammar.c"
)

add_custom_command(
  PRE_BUILD
  OUTPUT ${PROJECT_SOURCE_DIR}/src/grammar.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/grammar.y
  COMMENT "Generating grammar.c"
  COMMAND bison -d ${PROJECT_SOURCE_DIR}/src/grammar.y -o ${PROJECT_SOURCE_DIR}/src/grammar.c
)

add_custom_command(
  PRE_BUILD
  OUTPUT ${PROJECT_SOURCE_DIR}/src/lex.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/lex.l ${PROJECT_SOURCE_DIR}/src/grammar.c
  COMMENT "Generating lex.c"
  COMMAND flex -o ${PROJECT_SOURCE_DIR}/src/lex.c ${PROJECT_SOURCE_DIR}/src/lex.l
)

add_executable(fkparse ${fkparse_SRCS})
# target_link_libraries(fkparse Qt5::Core)
