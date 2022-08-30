#include "builtin.h"

static Proto f[] = {
  {"生成随机数", "math.random", TNumber, 2, {
    {"下界", TNumber, true, {.n = 1}},
    {"上界", TNumber, true, {.n = 10}}
  }},
  {"创建提示信息", "fkp.functions.buildPrompt", TString, 5, {
    {"文本", TString, false, {.s = NULL}},
    {"玩家1", TPlayer, true, {.s = "nil"}},
    {"玩家2", TPlayer, true, {.s = "nil"}},
    {"变量1", TAny, true, {.s = "nil"}},
    {"变量2", TAny, true, {.s = "nil"}},
  }},
  {"创建卡牌规则", "fkp.functions.buildPattern", TString, 3, {
    {"牌名表", TStringList, true, {.s = "nil"}},
    {"花色表", TStringList, true, {.s = "nil"}},
    {"点数表", TNumberList, true, {.s = "nil"}},
  }},
  {"创建虚拟牌", "fkp.functions.newVirtualCard", TCard, 5, {
    {"点数", TNumber, true, {.n = 0}},
    {"花色", TString, true, {.s = "no_suit"}},
    {"牌名", TString, true, {.s = "slash"}},
    {"子卡牌", TCardList, true, {.s = "nil"}},
    {"技能名", TString, true, {.s = ""}},
  }},
  {"创建卡牌移动信息", "fkp.functions.newMoveInfo", TString, 6, {
    {"卡牌列表", TCardList, false, {.s = NULL}},
    {"移动目标区域", TNumber, false, {.s = NULL}},
    {"移动目标角色", TPlayer, true, {.s = "nil"}},
    {"移牌原因", TNumber, true, {.n = 7}}, /* sgs.CardMoveReason_S_REASON_GOTCARD */
    {"技能名", TString, true, {.s = ""}},
    {"公开", TBool, true, {.n = true}},
  }},
  {"移动卡牌", "fkp.functions.moveCards", TNone, 1, {
    {"移牌信息列表", TStringList, false, {.s = NULL}},
  }},
  {"警告框", "sgs.Alert", TNone, 1, {
    {"信息", TAny, false, {.s = NULL}},
  }},
  {NULL, NULL, TNone, 0, {}}
};

void load_builtin_func() {
  loadmodule(f, NULL);
}
