set(fkparse_SRCS
  "main.c"
  "parse/lex.c"
  "parse/grammar.c"
  "parse/error.c"
  "fundamental/hash.c"
  "fundamental/list.c"
  "fundamental/symtab.c"
  "fundamental/object.c"
  "analyze/builtin.c"
  "analyze/generate.c"

  "analyze/builtin/action.c"
  "analyze/builtin/cards.c"
  "analyze/builtin/enum.c"
  "analyze/builtin/func.c"
  "analyze/builtin/getter.c"
  "analyze/builtin/interaction.c"
  "analyze/builtin/util.c"
)

add_custom_command(
  PRE_BUILD
  OUTPUT ${PROJECT_SOURCE_DIR}/src/parse/grammar.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/parse/grammar.y
  COMMENT "Generating grammar.c"
  COMMAND ${BISON} -d ${PROJECT_SOURCE_DIR}/src/parse/grammar.y -o ${PROJECT_SOURCE_DIR}/src/parse/grammar.c -Wcounterexamples
)

add_custom_command(
  PRE_BUILD
  OUTPUT ${PROJECT_SOURCE_DIR}/src/parse/lex.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/parse/lex.l ${PROJECT_SOURCE_DIR}/src/parse/grammar.c
  COMMENT "Generating lex.c"
  COMMAND ${FLEX} -o ${PROJECT_SOURCE_DIR}/src/parse/lex.c ${PROJECT_SOURCE_DIR}/src/parse/lex.l
)

add_executable(fkparse ${fkparse_SRCS})
