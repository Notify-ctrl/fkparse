#include "builtin.h"

static Proto f[] = {
  /* array operations */
  {"__prepend", "fkp.functions.prepend", TNone, 2, {
    {"array", TAny, false, {.s = NULL}},
    {"value", TAny, false, {.s = NULL}},
  }},
  {"__append", "fkp.functions.append", TNone, 2, {
    {"array", TAny, false, {.s = NULL}},
    {"value", TAny, false, {.s = NULL}},
  }},
  {"__removeOne", "fkp.functions.removeOne", TNone, 2, {
    {"array", TAny, false, {.s = NULL}},
    {"value", TAny, false, {.s = NULL}},
  }},
  {NULL, NULL, TNone, 0, {}}
};

static BuiltinVar v[] = {
  {"nil", "nil", TAny},
  {"不存在的", "nil", TAny},
  {"判定结构体", "judge", TAny},
  {"其他角色", "room:getOtherPlayers(player)", TPlayerList},
  {"使用牌信息", "data:toCardUse()", TAny},
  {NULL, NULL, TNone}
};

void load_builtin_util() {
  loadmodule(f, v);
}
