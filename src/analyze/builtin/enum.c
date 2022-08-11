#include "builtin.h"

static BuiltinVar v[] = {
  {"魏", "'wei'", TNumber},
  {"蜀", "'shu'", TNumber},
  {"吴", "'wu'", TNumber},
  {"群", "'qun'", TNumber},
  {"神", "'god'", TNumber},

  {"黑桃", "'spade'", TString},
  {"红桃", "'heart'", TString},
  {"梅花", "'club'", TString},
  {"方块", "'diamond'", TString},
  {"无花色", "'no_suit'", TString},
  {"黑色无花色", "'no_suit_black'", TString},
  {"红色无花色", "'no_suit_red'", TString},

  {"基本牌", "'basic'", TString},
  {"装备牌", "'equip'", TString},
  {"锦囊牌", "'trick'", TString},

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

  {"男性", "sgs.General_Male", TNumber},
  {"女性", "sgs.General_Female", TNumber},
  {"中性", "sgs.General_Neuter", TNumber},

  {"只放置顶部", "sgs.Room_GuanxingUpOnly", TNumber},
  {"顶部底部均放置", "sgs.Room_GuanxingBothSides", TNumber},
  {"只放置底部", "sgs.Room_GuanxingDownOnly", TNumber},

  /* card move reasons */
  {"因使用而移动", "sgs.CardMoveReason_S_REASON_USE", TNumber},
  {"因打出而移动", "sgs.CardMoveReason_S_REASON_RESPONSE", TNumber},
  {"因弃置而移动", "sgs.CardMoveReason_S_REASON_DISCARD", TNumber},
  {"因重铸而移动", "sgs.CardMoveReason_S_REASON_RECAST", TNumber},
  {"因拼点而移动", "sgs.CardMoveReason_S_REASON_PINDIAN", TNumber},
  {"因摸牌而移动", "sgs.CardMoveReason_S_REASON_DRAW", TNumber},
  {"因置入而移动", "sgs.CardMoveReason_S_REASON_PUT", TNumber},
  {"因交给而移动", "sgs.CardMoveReason_S_REASON_GIVE", TNumber},
  {"因换牌而移动", "sgs.CardMoveReason_S_REASON_SWAP", TNumber},

  {NULL, NULL, TNone}
};

void load_builtin_enum() {
  loadmodule(NULL, v);
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
