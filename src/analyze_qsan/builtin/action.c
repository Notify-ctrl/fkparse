#include "builtin.h"

static Proto f[] = {
  {"__drawCards", "fkp.functions.drawCards", TNone, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"数量", TNumber, false, {.s = NULL}}
  }},
  {"__loseHp", "fkp.functions.loseHp", TNone, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"数量", TNumber, false, {.s = NULL}}
  }},
  {"__loseMaxHp", "fkp.functions.loseMaxHp", TNone, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"数量", TNumber, false, {.s = NULL}}
  }},
  {"__damage", "fkp.functions.damage", TNone, 6, {
    {"伤害来源", TPlayer, true, {.s = "nil"}},
    {"伤害目标", TPlayer, false, {.s = NULL}},
    {"伤害值", TNumber, false, {.s = NULL}},
    {"伤害属性", TNumber, true, {.n = 0}},  /* 无属性 */
    {"造成伤害的牌", TCard, true, {.s = "nil"}},
    {"造成伤害的原因", TString, true, {.s = ""}}
  }},
  {"__recover", "fkp.functions.recover", TNone, 4, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"数量", TNumber, false, {.s = NULL}},
    {"回复来源", TPlayer, true, {.s = "nil"}},
    {"回复的牌", TCard, true, {.s = "nil"}},
  }},
  {"__recoverMaxHp", "fkp.functions.recoverMaxHp", TNone, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"数量", TNumber, false, {.s = NULL}}
  }},
  {"__acquireSkill", "fkp.functions.acquireSkill", TNone, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能", TString, false, {.s = NULL}}
  }},
  {"__loseSkill", "fkp.functions.loseSkill", TNone, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能", TString, false, {.s = NULL}}
  }},
  {"__addMark", "fkp.functions.addMark", TNone, 4, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"标记", TString, false, {.s = NULL}},
    {"数量", TNumber, false, {.s = NULL}},
    {"隐藏", TBool, true, {.n = false}},
  }},
  {"__loseMark", "fkp.functions.loseMark", TNone, 4, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"标记", TString, false, {.s = NULL}},
    {"数量", TNumber, false, {.s = NULL}},
    {"隐藏", TBool, true, {.n = false}},
  }},
  {"__obtainCard", "fkp.functions.obtainCard", TNone, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"卡牌", TCard, false, {.s = NULL}},
    /* {"获得的原因", TString, true, {.s = ""}}, */
    {"公开", TBool, true, {.n = true}},
  }},
  {"__throwCardsBySkill", "fkp.functions.throwCardsBySkill", TNone, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"卡牌列表", TCardList, false, {.s = NULL}},
    {"技能名", TString, false, {.s = NULL}},
  }},
  {"__broadcastSkillInvoke", "fkp.functions.broadcastSkillInvoke", TNone, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能名", TString, false, {.s = NULL}},
    {"音频编号", TNumber, true, {.n = -1}},
  }},
  {"__judge", "fkp.functions.judge", TCard, 5, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能名", TString, true, {.s = ""}},
    {"判定规则", TString, true, {.s = "."}},
    {"希望判定中", TBool, true, {.n = true}},
    {"播放动画效果", TBool, true, {.n = true}},
  }},
  {"__swapPile", "fkp.functions.swapPile", TNone, 1, {
    {"玩家", TPlayer, false, {.s = NULL}}
  }},
  {"__changeHero", "fkp.functions.changeHero", TNone, 6, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"新将领", TString, false, {.s = NULL}},
    {"是否满状态", TBool, true, {.n = true}},
    {"是否以开始游戏状态变身", TBool, true, {.n = true}},
    {"是否是变更副将", TBool, true, {.n = false}},
    {"是否发送信息", TBool, true, {.n = true}},
  }},
  {"__swapSeat", "fkp.functions.swapSeat", TNone, 2, {
    {"玩家A", TPlayer, false, {.s = NULL}},
    {"玩家B", TPlayer, false, {.s = NULL}},
  }},
  {"__getNCards", "fkp.functions.getNCards", TCardList, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"获得牌的数量", TNumber, false, {.s = NULL}},
    {"是否不放回", TBool, true, {.n = true}},
  }},
  {"__retrial", "fkp.functions.retrial", TNone, 5, {
    {"改判牌", TCard, false, {.s = NULL}},
    {"玩家", TPlayer, false, {.s = NULL}},
    {"判定结构体", TAny, true, {.s = "判定结构体"}},
    {"技能名", TString, true, {.s = ""}},
    {"是否交换", TBool, true, {.n = false}},
  }},
  {"__chat", "fkp.functions.chat", TNone, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"聊天句子", TAny, false, {.s = NULL}},
  }},
  {"__sendlog", "fkp.functions.sendlog", TNone, 7, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"战报", TString, false, {.s = NULL}},
    {"%from", TPlayer, true, {.s = "nil"}},
    {"%to", TPlayerList, true, {.s = "nil"}},
    {"%card", TCard, true, {.s = "nil"}},
    {"%arg", TAny, true, {.s = "nil"}},
    {"%arg2", TAny, true, {.s = "nil"}},
  }},
  {"__throwCards", "fkp.functions.throwCards", TNone, 4, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"来源", TPlayer, true, {.s = "nil"}},
    {"技能名", TString, true, {.s = ""}},
    {"卡牌列表", TCardList, false, {.s = NULL}},
  }},
  {"__giveCards", "fkp.functions.giveCards", TNone, 5, {
    {"目标", TPlayer, false, {.s = NULL}},
    {"来源", TPlayer, false, {.s = NULL}},
    {"卡牌列表", TCardList, false, {.s = NULL}},
    {"技能名", TString, true, {.s = ""}},
    {"公开", TBool, true, {.n = false}},
  }},
  {"__pindian", "fkp.functions.pindian", TPindian, 3, {
    {"目标", TPlayer, false, {.s = NULL}},
    {"来源", TPlayer, false, {.s = NULL}},
    {"技能名", TString, true, {.s = ""}},
  }},
  {"__swapCards", "fkp.functions.swapCards", TNone, 4, {
    {"目标", TPlayer, false, {.s = NULL}},
    {"来源", TPlayer, false, {.s = NULL}},
    {"技能名", TString, true, {.s = ""}},
    {"区域", TNumber, true, {.n = 0}},  /* sgs.Player_PlaceHand */
  }},
  {"__turnOver", "fkp.functions.turnOver", TNone, 1, {
    {"玩家", TPlayer, false, {.s = NULL}},
  }},
  {"__playExtraTurn", "fkp.functions.playExtraTurn", TNone, 1, {
    {"玩家", TPlayer, false, {.s = NULL}},
  }},
  {"__skipPhase", "fkp.functions.skipPhase", TNone, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"阶段", TNumber, false, {.s = NULL}},
  }},
  {"__inMyAttackRange", "fkp.functions.inMyAttackRange", TBool, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"目标", TPlayer, false, {.s = NULL}},
    {"距离修正", TNumber, true, {.n = 0}}
  }},
  {"__distanceTo", "fkp.functions.distanceTo", TNumber, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"目标", TPlayer, false, {.s = NULL}},
    {"距离修正", TNumber, true, {.n = 0}},
  }},
  {"__isAdjacentTo", "fkp.functions.isAdjacentTo", TBool, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"目标", TPlayer, false, {.s = NULL}},
  }},
  {"__jinknum", "fkp.functions.jinknum", TNone, 4, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"用牌信息", TAny, true, {.s = "使用牌信息"}},
    {"需闪数", TNumber, false, {.s = NULL}},
    {"目标", TPlayer, false, {.s = NULL}},
  }},
  {NULL, NULL, TNone, 0, {}}
};

void load_builtin_action() {
  loadmodule(f, NULL);
}
