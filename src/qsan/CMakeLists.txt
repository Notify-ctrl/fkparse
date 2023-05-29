set(fkparse_qsan_SRCS
  "qsan.c"
  "generate.c"

  "builtin/action.c"
  "builtin/cards.c"
  "builtin/enum.c"
  "builtin/func.c"
  "builtin/getter.c"
  "builtin/interaction.c"
  "builtin/util.c"
)

target_sources(fkparse PRIVATE ${fkparse_qsan_SRCS})
target_include_directories(fkparse PRIVATE .)
