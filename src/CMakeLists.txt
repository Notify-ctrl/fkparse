set(fkparse_SRCS
  "main.c"
  "parse/lex.c"
  "parse/grammar.c"
  "parse/error.c"
  "fundamental/hash.c"
  "fundamental/list.c"
  "fundamental/symtab.c"
  "fundamental/object.c"
  "fundamental/builtin.c"
)

target_sources(fkparse PRIVATE ${fkparse_SRCS})
target_include_directories(fkparse PRIVATE .)
target_include_directories(fkparse PRIVATE parse)
target_include_directories(fkparse PRIVATE fundamental)

add_subdirectory(qsan)
add_subdirectory(noname)

set_target_properties(fkparse
  PROPERTIES
  C_VISIBILITY_PRESET hidden
)
