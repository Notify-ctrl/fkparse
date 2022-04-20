set(fkparse_SRCS
  "main.c"
  "parse/ast.c"
  "parse/lex.c"
  "parse/grammar.c"
  "analyze/analyzer.c"
  "analyze/skill.c"
  "analyze/statement.c"
  "analyze/symtab.c"
  "analyze/translations.c"
)

add_custom_command(
  PRE_BUILD
  OUTPUT ${PROJECT_SOURCE_DIR}/src/parse/grammar.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/parse/grammar.y
  COMMENT "Generating grammar.c"
  COMMAND bison -d ${PROJECT_SOURCE_DIR}/src/parse/grammar.y -o ${PROJECT_SOURCE_DIR}/src/parse/grammar.c -Wcounterexamples
)

add_custom_command(
  PRE_BUILD
  OUTPUT ${PROJECT_SOURCE_DIR}/src/parse/lex.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/parse/lex.l ${PROJECT_SOURCE_DIR}/src/parse/grammar.c
  COMMENT "Generating lex.c"
  COMMAND flex -o ${PROJECT_SOURCE_DIR}/src/parse/lex.c ${PROJECT_SOURCE_DIR}/src/parse/lex.l
)

add_executable(fkparse ${fkparse_SRCS})
