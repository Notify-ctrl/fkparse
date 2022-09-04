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
  {NULL, NULL, TNone}
};

void load_builtin_util() {
  loadmodule(f, v);
  /* fkp.cost是可读写的全局数据，所以要特意把reserved设为false */
  /* 它仅用于为消耗函数提供额外返回值 */
  sym_new_entry("消耗数据", TAny, "fkp.cost", false);
}
