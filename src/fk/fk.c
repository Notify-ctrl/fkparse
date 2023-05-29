#include "fk.h"
#include "qsan.h"

static void (*init_funcs[])() = {
  qsan_load_action,
  fk_load_cards,
  fk_load_enum,
  qsan_load_func,
  qsan_load_getter,
  qsan_load_interaction,
  fk_load_util,
  NULL
};

void fk_load()
{
  for (int i = 0; ; i++) {
    if (init_funcs[i] == NULL)
      break;
    init_funcs[i]();
  }
}
