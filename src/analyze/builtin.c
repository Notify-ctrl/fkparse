#include "structs.h"
#include "ast.h"
#include "object.h"
#include "main.h"

struct ProtoArg {
  const char *name;
  ExpVType argtype;
  bool have_default;
  union {
    long long n;
    const char *s;
  } d;
};

typedef struct {
  const char *dst;
  const char *src;
  ExpVType rettype;
  int argcount;
  struct ProtoArg args[10];
} Proto;

static Proto builtin_func[] = {
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
  {"__at", "fkp.functions.at", TAny, 2, {
    {"array", TAny, false, {.s = NULL}},
    {"index", TNumber, false, {.s = NULL}},
  }},

  /* Built-in functions for actions */
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
  {"__getMark", "fkp.functions.getMark", TNumber, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"标记", TString, false, {.s = NULL}},
    {"隐藏", TBool, true, {.n = false}},
  }},
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
  {"__obtainCard", "fkp.functions.obtainCard", TNone, 4, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"卡牌", TCard, false, {.s = NULL}},
    {"获得的原因", TString, true, {.s = ""}},
    {"公开", TBool, true, {.n = true}},
  }},
  {"__hasSkill", "fkp.functions.hasSkill", TBool, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能", TString, false, {.s = NULL}},
  }},
  {"__throwCardsBySkill", "fkp.functions.throwCardsBySkill", TNone, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"卡牌列表", TCardList, false, {.s = NULL}},
    {"技能名", TString, false, {.s = NULL}},
  }},
  {"__getUsedTimes", "fkp.functions.getUsedTimes", TNumber, 2, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能名", TString, false, {.s = NULL}},
  }},
  {"__broadcastSkillInvoke", "fkp.functions.broadcastSkillInvoke", TNone, 3, {
    {"玩家", TPlayer, false, {.s = NULL}},
    {"技能名", TString, false, {.s = NULL}},
    {"音频编号", TNumber, true, {.n = -1}},
  }},
  {"__askForDiscard", "fkp.functions.askForDiscard", TCard, 8, {
    {"目标", TPlayer, false, {.s = NULL}},
    {"技能名", TString, true, {.s = ""}},
    {"要求弃置数量", TNumber, false, {.s = NULL}},
    {"最小弃置数量", TNumber, true, {.n = -1}},
    {"可以点取消", TBool, true, {.n = false}},
    {"可以弃装备", TBool, true, {.n = true}},
    {"提示信息", TString, true, {.s = ""}},
    {"卡牌正则", TString, true, {.s = "."}},
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
    {"是否发送信息", TBool, true, {.n=true}},
  }},
  {"__swapSeat", "fkp.functions.swapSeat", TNone, 2, {
    {"玩家A", TPlayer, false, {.s = NULL}},
    {"玩家B", TPlayer, false, {.s = NULL}},
  }},
  {NULL, NULL, TNone, 0, {}}
};

static struct {
  char *dst;
  char *src;
  int type;
} reserved[] = {
  {"nil", "nil", TAny},

  {"魏", "'wei'", TNumber},
  {"蜀", "'shu'", TNumber},
  {"吴", "'wu'", TNumber},
  {"群", "'qun'", TNumber},
  {"神", "'god'", TNumber},

  {"黑桃", "sgs.Card_Spade", TNumber},
  {"红桃", "sgs.Card_Heart", TNumber},
  {"梅花", "sgs.Card_Club", TNumber},
  {"方块", "sgs.Card_Diamond", TNumber},
  {"无花色", "sgs.Card_NoSuit", TNumber},

  {"基本牌", "sgs.Card_TypeBasic", TNumber},
  {"装备牌", "sgs.Card_TypeEquip", TNumber},
  {"锦囊牌", "sgs.Card_TypeTrick", TNumber},
  {"技能卡", "sgs.Card_TypeSkill", TNumber},

  {"手牌区", "sgs.Player_PlaceHand", TNumber},
  {"装备区", "sgs.Player_PlaceEquip", TNumber},
  {"判定区", "sgs.Player_PlaceDelayedTrick", TNumber},
  {"武将牌上", "sgs.Player_PlaceSpecial", TNumber},
  {"弃牌堆", "sgs.Player_DiscardPile", TNumber},
  {"牌堆", "sgs.Player_DrawPile", TNumber},
  {"处理区", "sgs.Player_PlaceTable", TNumber},

  {"无属性", "sgs.DamageStruct_Normal", TNumber},
  {"火属性", "sgs.DamageStruct_Fire", TNumber},
  {"雷属性", "sgs.DamageStruct_Thunder", TNumber},

  {"锁定技", "sgs.Skill_Compulsory", TNumber},
  {"普通技", "sgs.Skill_NotFrequent", TNumber},
  {"默认技", "sgs.Skill_Frequent", TNumber},
  {"觉醒技", "sgs.Skill_Wake", TNumber},
  {"限定技", "sgs.Skill_Limited", TNumber},

  {"开始阶段", "sgs.Player_RoundStart", TNumber},
  {"准备阶段", "sgs.Player_Start", TNumber},
  {"判定阶段", "sgs.Player_Judge", TNumber},
  {"摸牌阶段", "sgs.Player_Draw", TNumber},
  {"出牌阶段", "sgs.Player_Play", TNumber},
  {"弃牌阶段", "sgs.Player_Discard", TNumber},
  {"结束阶段", "sgs.Player_Finish", TNumber},
  {"回合外", "sgs.Player_NotActive", TNumber},

  {"主公", "sgs.Player_Lord", TNumber},
  {"忠臣", "sgs.Player_Loyalist", TNumber},
  {"反贼", "sgs.Player_Rebel", TNumber},
  {"内奸", "sgs.Player_Renegade", TNumber},

  {"杀", "'slash'", TString},
  {"闪", "'jink'", TString},
  {"桃", "'peach'", TString},
  {"酒", "'analeptic'", TString},
  {"过河拆桥", "'dismantlement'", TString},
  {"顺手牵羊", "'snatch'", TString},
  {"决斗", "'duel'", TString},
  {"借刀杀人", "'collateral'", TString},
  {"无中生有", "'ex_nihilo'", TString},
  {"无懈可击", "'nullification'", TString},
  {"南蛮入侵", "'savage_assault'", TString},
  {"万箭齐发", "'archery_attack'", TString},
  {"桃园结义", "'god_salvation'", TString},
  {"五谷丰登", "'amazing_grace'", TString},
  {"闪电", "'lightning'", TString},
  {"乐不思蜀", "'indulgence'", TString},
  {"诸葛连弩", "'crossbow'", TString},
  {"青釭剑", "'qinggang_sword'", TString},
  {"寒冰剑", "'ice_sword'", TString},
  {"雌雄双股剑", "'double_sword'", TString},
  {"青龙偃月刀", "'blade'", TString},
  {"丈八蛇矛", "'spear'", TString},
  {"贯石斧", "'axe'", TString},
  {"方天画戟", "'halberd'", TString},
  {"麒麟弓", "'kylin_bow'", TString},
  {"八卦阵", "'eight_diagram'", TString},
  {"仁王盾", "'renwang_shield'", TString},
  {"的卢", "'dilu'", TString},
  {"绝影", "'jueying'", TString},
  {"爪黄飞电", "'zhuahuangfeidian'", TString},
  {"赤兔", "'chitu'", TString},
  {"大宛", "'dayuan'", TString},
  {"紫骍", "'zixing'", TString},
  {"雷杀", "'thunder_slash'", TString},
  {"火杀", "'fire_slash'", TString},
  {"古锭刀", "'guding_blade'", TString},
  {"藤甲", "'vine'", TString},
  {"兵粮寸断", "'supply_shortage'", TString},
  {"铁索连环", "'iron_chain'", TString},
  {"白银狮子", "'sliver_lion'", TString},
  {"火攻", "'fire_attack'", TString},
  {"朱雀羽扇", "'fan'", TString},
  {"骅骝", "'hualiu'", TString},

  {"男性", "sgs.General_Male", TNumber},
  {"女性", "sgs.General_Female", TNumber},
  {"中性", "sgs.General_Neuter", TNumber},

  {"其他角色", "room:getOtherPlayers(player)", TPlayerList},

  {NULL, NULL, TNone}
};

void sym_init() {
  if (builtin_symtab != NULL) return;
  builtin_symtab = hash_new();
  symtab_stack = stack_new();
  stack_push(symtab_stack, cast(Object *, builtin_symtab));
  current_tab = builtin_symtab;
  last_lookup_tab = NULL;

  for (int i = 0; ; i++) {
    Proto *p = &builtin_func[i];
    if (p->dst == NULL) break;

    FuncdefObj *def = malloc(sizeof(FuncdefObj));
    def->objtype = Obj_Funcdef;
    def->funcname = strdup(p->src);
    sym_new_entry(p->dst, TFunc, cast(const char *, def), true);
    def->rettype = p->rettype;
    def->funcbody = NULL;

    List *l = list_new();
    for (int i = 0; i < p->argcount; i++) {
      struct ProtoArg *arg = &p->args[i];
      DefargObj *defarg = malloc(sizeof(DefargObj));
      defarg->objtype = Obj_Defarg;
      defarg->name = strdup(arg->name);
      defarg->type = arg->argtype;

      if (!arg->have_default) {
        defarg->d = NULL;
      } else {
        ExpressionObj *e = malloc(sizeof(ExpressionObj));
        e->objtype = Obj_Expression;
        e->valuetype = arg->argtype;
        e->optype = 0;
        e->strvalue = NULL;
        e->varValue = NULL;
        e->func = NULL;
        e->array = NULL;
        e->oprand1 = NULL;
        e->oprand2 = NULL;
        e->bracketed = false;
        e->param_name = strdup(arg->name);

        VarObj *v;
        switch (arg->argtype) {
        case TNumber:
          e->exptype = ExpNum;
          e->value = arg->d.n;
          break;
        case TString:
          e->exptype = ExpStr;
          e->strvalue = strdup(arg->d.s);
          break;
        case TPlayer:
        case TCard:
        case TAny:
          e->exptype = ExpVar;
          v = malloc(sizeof(VarObj));
          v->objtype = Obj_Var;
          v->name = strdup(arg->d.s);
          v->type = arg->argtype;
          v->obj = NULL;
          e->varValue = v;
          break;
        case TBool:
          e->exptype = ExpBool;
          e->value = arg->d.n;
          break;
        default:
          fprintf(stderr, "Error: unknown builtin function argtype %d\n",
                  arg->argtype);
          exit(1);
        }
        defarg->d = e;
      }
      list_append(l, cast(Object *, defarg));
    }
    def->params = l;
  }

  for (int i = 0; ; i++) {
    if (reserved[i].dst == NULL) break;
    sym_new_entry(reserved[i].dst, reserved[i].type, reserved[i].src, true);
  }
}

char *event_table[] = {
  "sgs.NonTrigger",

  "sgs.GameStart",
  "sgs.TurnStart",
  "sgs.EventPhaseStart",
  "sgs.EventPhaseProceeding",
  "sgs.EventPhaseEnd",
  "sgs.EventPhaseChanging",
  "sgs.EventPhaseSkipping",

  "sgs.DrawNCards",
  "sgs.AfterDrawNCards",
  "sgs.DrawInitialCards",
  "sgs.AfterDrawInitialCards",

  "sgs.PreHpRecover",
  "sgs.HpRecover",
  "sgs.PreHpLost",
  "sgs.HpLost",
  "sgs.HpChanged",
  "sgs.MaxHpChanged",

  "sgs.EventLoseSkill",
  "sgs.EventAcquireSkill",

  "sgs.StartJudge",
  "sgs.AskForRetrial",
  "sgs.FinishRetrial",
  "sgs.FinishJudge",

  "sgs.PindianVerifying",
  "sgs.Pindian",

  "sgs.TurnedOver",
  "sgs.ChainStateChanged",

  "sgs.ConfirmDamage",
  "sgs.Predamage",
  "sgs.DamageForseen",
  "sgs.DamageCaused",
  "sgs.DamageInflicted",
  "sgs.PreDamageDone",
  "sgs.DamageDone",
  "sgs.Damage",
  "sgs.Damaged",
  "sgs.DamageComplete",

  "sgs.EnterDying",
  "sgs.Dying",
  "sgs.QuitDying",
  "sgs.AskForPeaches",
  "sgs.AskForPeachesDone",
  "sgs.Death",
  "sgs.BuryVictim",
  "sgs.BeforeGameOverJudge",
  "sgs.GameOverJudge",
  "sgs.GameFinished",

  "sgs.SlashEffected",
  "sgs.SlashProceed",
  "sgs.SlashHit",
  "sgs.SlashMissed",

  "sgs.JinkEffect",
  "sgs.NullificationEffect",

  "sgs.CardAsked",
  "sgs.PreCardResponded",
  "sgs.CardResponded",
  "sgs.BeforeCardsMove",
  "sgs.CardsMoveOneTime",

  "sgs.PreCardUsed",
  "sgs.CardUsed",
  "sgs.TargetSpecifying",
  "sgs.TargetConfirming",
  "sgs.TargetSpecified",
  "sgs.TargetConfirmed",
  "sgs.CardEffect",
  "sgs.CardEffected",
  "sgs.PostCardEffected",
  "sgs.CardFinished",
  "sgs.TrickCardCanceling",
  "sgs.TrickEffect",

  "sgs.ChoiceMade",

  "sgs.StageChange",
  "sgs.FetchDrawPileCard",
  "sgs.Debut",

  "sgs.TurnBroken",

  "sgs.NumOfEvents"
};
