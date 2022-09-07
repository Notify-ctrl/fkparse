set(fkparse_noname_SRCS
  "noname.c"
  "generate.c"

  "builtin/cards.c"
  "builtin/enum.c"
  "builtin/util.c"
)

target_sources(fkparse PRIVATE ${fkparse_noname_SRCS})
target_include_directories(fkparse PRIVATE .)
