set(fkparse_SRCS
  "main.c"
  "parse/lex.c"
  "parse/grammar.c"
  "parse/error.c"
  "fundamental/hash.c"
  "fundamental/list.c"
  "fundamental/symtab.c"
  "fundamental/object.c"
)

target_sources(fkparse PRIVATE ${fkparse_SRCS})
target_include_directories(fkparse PRIVATE .)
target_include_directories(fkparse PRIVATE parse)
target_include_directories(fkparse PRIVATE fundamental)

add_subdirectory(analyze_qsan)

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

set_target_properties(fkparse
  PROPERTIES
  C_VISIBILITY_PRESET hidden
)
