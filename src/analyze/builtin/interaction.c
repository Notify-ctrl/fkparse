#include "builtin.h"

static Proto f[] = {
  {"__askForChoice", "fkp.functions.askForChoice", TString, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"选项列表", TStringList, false, {.s = NULL}},
    {"选择的原因", TString, true, {.s = ""}},
  }},
  {"__askForPlayerChosen", "fkp.functions.askForPlayerChosen", TPlayer, 6, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"可选列表", TPlayerList, false, {.s = NULL}},
    {"选择的原因", TString, true, {.s = ""}},
    {"提示框文本", TString, true, {.s = ""}},
    {"可以点取消", TBool, true, {.n = true}},
    {"提示技能发动", TBool, true, {.n = false}},
  }},
  {"__askForSkillInvoke", "fkp.functions.askForSkillInvoke", TBool, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能", TString, false, {.s = NULL}},
  }},
  {"__askForDiscard", "fkp.functions.askForDiscard", TCardList, 8, {
    {"目标", TPlayer, false, {.s = NULL}},
    {"技能名", TString, true, {.s = ""}},
    {"要求弃置数量", TNumber, false, {.s = NULL}},
    {"最小弃置数量", TNumber, true, {.n = -1}},
    {"可以点取消", TBool, true, {.n = false}},
    {"可以弃装备", TBool, true, {.n = true}},
    {"提示信息", TString, true, {.s = ""}},
    {"弃牌规则", TString, true, {.s = "."}},
  }},
  {"__askForGuanxing", "fkp.functions.askForGuanxing", TNone, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"参与观星的牌", TCardList, false, {.s = NULL}},
    {"观星类型", TNumber, true, {.n = 0}}, // 0 is GuanxingBothSides
  }},
  {"__askForCard", "fkp.functions.askForCard", TCard, 4, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"选牌规则", TString, true, {.s = "."}},
    {"提示", TString, true, {.s = ""}},
    {"技能名", TString, true, {.s = ""}},
  }},
  {"__askRespondForCard", "fkp.functions.askRespondForCard", TCard, 5, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"选牌规则", TString, true, {.s = "."}},
    {"提示", TString, true, {.s = ""}},
    {"是否为改判", TBool, true, {.n = false}},
    {"技能名", TString, true, {.s = ""}},
  }},
  {"__askUseForCard", "fkp.functions.askUseForCard", TCard, 5, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"选牌规则", TString, true, {.s = "."}},
    {"提示", TString, true, {.s = ""}},
    {"目标", TPlayer, true, {.s = "nil"}},
    {"技能名", TString, true, {.s = ""}},
  }},
  {"__askForCardChosen", "fkp.functions.askForCardChosen", TCard, 5, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"被选牌者", TPlayer, false, {.s = NULL}},
    {"位置", TNumberList, true, {.s = "nil"}},
    {"原因", TString, true, {.s = ""}},
    {"是否可见手牌", TBool, true, {.n = false}},
  }},

  {NULL, NULL, TNone, 0, {}}
};

void load_builtin_interaction() {
  loadmodule(f, NULL);
}
