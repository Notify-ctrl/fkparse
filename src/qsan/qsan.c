#include "qsan.h"

static void (*init_funcs[])() = {
  qsan_load_action,
  qsan_load_cards,
  qsan_load_enum,
  qsan_load_func,
  qsan_load_getter,
  qsan_load_interaction,
  qsan_load_util,
  NULL
};

void qsan_load()
{
  for (int i = 0; ; i++) {
    if (init_funcs[i] == NULL)
      break;
    init_funcs[i]();
  }
}
