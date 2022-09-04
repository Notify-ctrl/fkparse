#include "builtin.h"

static BuiltinVar v[] = {
  {"所有角色", "fkp.functions.getAllPlayers()", TPlayerList},
  {NULL, NULL, TNone},
};

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
  {"__getOtherPlayers", "fkp.functions.getOtherPlayers", TPlayerList, 1, {
    {"排除的玩家", TPlayer, false, {.s = NULL}},
  }},
  {"__getPile", "fkp.functions.getPile", TCardList, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"牌堆名", TString, false, {.s = NULL}},
  }},
  {"__getSkillUsedTimes", "fkp.functions.getSkillUsedTimes", TNumber, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能名", TString, false, {.s = NULL}},
    {"格局", TNumber, false, {.s = NULL}},
  }},
  {NULL, NULL, TNone, 0, {}}
};

void load_builtin_getter() {
  loadmodule(f, v);
}
