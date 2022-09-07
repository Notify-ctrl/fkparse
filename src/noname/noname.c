#include "noname.h"
#include "qsan.h"

static void (*init_funcs[])() = {
  qsan_load_action,
  noname_load_cards,
  noname_load_enum,
  qsan_load_func,
  qsan_load_getter,
  qsan_load_interaction,
  noname_load_util,
  NULL
};

void noname_load()
{
  for (int i = 0; ; i++) {
    if (init_funcs[i] == NULL)
      break;
    init_funcs[i]();
  }
}
