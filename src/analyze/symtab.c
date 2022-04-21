#include "analyzer.h"

#define NHASH 9997
static struct symbol symtab[NHASH];

/* symbol table */
/* hash a symbol */
static unsigned
symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while ((c = *(sym++))) hash = hash * 9 ^ c;

  return hash;
}

struct symbol *lookup(char *sym)
{
  struct symbol *sp = &symtab[symhash(sym) % NHASH];
  int scount = NHASH;		/* how many have we looked at */

  while(--scount >= 0) {
    if(sp->name && !strcmp(sp->name, sym)) { return sp; }

    if(!sp->name) { /* new entry */
      sp->name = sym;
      sp->type = TNone;
      return sp;
    }

    if(++sp >= symtab+NHASH) sp = symtab; /* try the next entry */
  }
  yyerror("symbol table overflow\n");
  abort(); /* tried them all, table is full */

}

static struct {
  char *dst;
  char *src;
  int type;
} reserved[] = {
  {"你", "player", TPlayer},

  {"魏", "\"wei\"", TNumber},
  {"蜀", "\"shu\"", TNumber},
  {"吴", "\"wu\"", TNumber},
  {"群", "\"qun\"", TNumber},
  {"神", "\"god\"", TNumber},

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

  {"杀", "\"slash\"", TString},
  {"闪", "\"jink\"", TString},
  {"桃", "\"peach\"", TString},
  {"酒", "\"analeptic\"", TString},
  {"过河拆桥", "\"dismantlement\"", TString},
  {"顺手牵羊", "\"snatch\"", TString},
  {"决斗", "\"duel\"", TString},
  {"借刀杀人", "\"collateral\"", TString},
  {"无中生有", "\"ex_nihilo\"", TString},
  {"无懈可击", "\"nullification\"", TString},
  {"南蛮入侵", "\"savage_assault\"", TString},
  {"万箭齐发", "\"archery_attack\"", TString},
  {"桃园结义", "\"god_salvation\"", TString},
  {"五谷丰登", "\"amazing_grace\"", TString},
  {"闪电", "\"lightning\"", TString},
  {"乐不思蜀", "\"indulgence\"", TString},
  {"诸葛连弩", "\"crossbow\"", TString},
  {"青釭剑", "\"qinggang_sword\"", TString},
  {"寒冰剑", "\"ice_sword\"", TString},
  {"雌雄双股剑", "\"double_sword\"", TString},
  {"青龙偃月刀", "\"blade\"", TString},
  {"丈八蛇矛", "\"spear\"", TString},
  {"贯石斧", "\"axe\"", TString},
  {"方天画戟", "\"halberd\"", TString},
  {"麒麟弓", "\"kylin_bow\"", TString},
  {"八卦阵", "\"eight_diagram\"", TString},
  {"仁王盾", "\"renwang_shield\"", TString},
  {"的卢", "\"dilu\"", TString},
  {"绝影", "\"jueying\"", TString},
  {"爪黄飞电", "\"zhuahuangfeidian\"", TString},
  {"赤兔", "\"chitu\"", TString},
  {"大宛", "\"dayuan\"", TString},
  {"紫骍", "\"zixing\"", TString},
  {"雷杀", "\"thunder_slash\"", TString},
  {"火杀", "\"fire_slash\"", TString},
  {"古锭刀", "\"guding_blade\"", TString},
  {"藤甲", "\"vine\"", TString},
  {"兵粮寸断", "\"supply_shortage\"", TString},
  {"铁索连环", "\"iron_chain\"", TString},
  {"白银狮子", "\"sliver_lion\"", TString},
  {"火攻", "\"fire_attack\"", TString},
  {"朱雀羽扇", "\"fan\"", TString},
  {"骅骝", "\"hualiu\"", TString},

  {"男性", "sgs.General_Male", TNumber},
  {"女性", "sgs.General_Female", TNumber},
  {"中性", "sgs.General_Neuter", TNumber},

  {NULL, NULL, TNone}
};

int isReserved(char *k) {
  int ret = 0;
  for (int i = 0; ; i++) {
    if (reserved[i].dst == NULL) break;
    if (!strcmp(k, reserved[i].dst)) {
      ret = 1;
      break;
    }
  }
  return ret;
}

int analyzeReserved(char *k) {
  int ret = TNone;
  for (int i = 0; ; i++) {
    if (reserved[i].dst == NULL) break;
    if (!strcmp(k, reserved[i].dst)) {
      fprintf(yyout, "%s", reserved[i].src);
      ret = reserved[i].type;
      return ret;
    }
  }
  fprintf(stderr, "错误：未在预定义符号表中发现\"%s\"，请检查\n", k);
  exit(1);
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
