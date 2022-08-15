#include "builtin.h"

static Proto f[] = {
  {"__getMark", "fkp.functions.getMark", TNumber, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"标记", TString, false, {.s = NULL}},
    {"隐藏", TBool, true, {.n = false}},
  }},
  {"__hasSkill", "fkp.functions.hasSkill", TBool, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能", TString, false, {.s = NULL}},
  }},
  {"__getUsedTimes", "fkp.functions.getUsedTimes", TNumber, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能名", TString, false, {.s = NULL}},
  }},
  {NULL, NULL, TNone, 0, {}}
};

void load_builtin_getter() {
  loadmodule(f, NULL);
}
