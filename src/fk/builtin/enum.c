#include "fk.h"

static BuiltinVar v[] = {
  {"魏", "'wei'", TNumber},
  {"蜀", "'shu'", TNumber},
  {"吴", "'wu'", TNumber},
  {"群", "'qun'", TNumber},
  {"神", "'god'", TNumber},
  {"Wei", "'wei'", TNumber},
  {"Shu", "'shu'", TNumber},
  {"Wu", "'wu'", TNumber},
  {"Qun", "'qun'", TNumber},
  {"God", "'god'", TNumber},

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

  {"手牌区", "Card.PlayerHand", TNumber},
  {"装备区", "Card.PlayerEquip", TNumber},
  {"判定区", "Card.PlayerJudge", TNumber},
  {"武将牌上", "Card.PlayerSpecial", TNumber},
  {"弃牌堆", "Card.DiscardPile", TNumber},
  {"牌堆", "Card.DrawPile", TNumber},
  {"处理区", "Card.Processing", TNumber},

  {"无属性", "fk.NormalDamage", TNumber},
  {"火属性", "fk.FireDamage", TNumber},
  {"雷属性", "fk.ThunderDamage", TNumber},

  {"锁定技", "Skill.Compulsory", TNumber},
  {"普通技", "Skill.NotFrequent", TNumber},
  {"默认技", "Skill.Frequent", TNumber},
  {"觉醒技", "Skill.Wake", TNumber},
  {"限定技", "Skill.Limited", TNumber},
  {"Compulsory", "Skill.Compulsory", TNumber},
  {"NotFrequent", "Skill.NotFrequent", TNumber},
  {"Frequent", "Skill.Frequent", TNumber},
  {"Wake", "Skill.Wake", TNumber},
  {"Limited", "Skill.Limited", TNumber},

  {"开始阶段", "Player.RoundStart", TNumber},
  {"准备阶段", "Player.Start", TNumber},
  {"判定阶段", "Player.Judge", TNumber},
  {"摸牌阶段", "Player.Draw", TNumber},
  {"出牌阶段", "Player.Play", TNumber},
  {"弃牌阶段", "Player.Discard", TNumber},
  {"结束阶段", "Player.Finish", TNumber},
  {"回合外", "Player.NotActive", TNumber},

  {"主公", "'lord'", TString},
  {"忠臣", "'loyalist'", TString},
  {"反贼", "'rebel'", TString},
  {"内奸", "'renegade'", TString},

  {"男性", "General.Male", TNumber},
  {"女性", "General.Female", TNumber},
  {"中性", "General.Neuter", TNumber},
  {"Male", "General.Male", TNumber},
  {"Female", "General.Female", TNumber},
  {"Neuter", "General.Neuter", TNumber},

  {"只放置顶部", "fk.Room_GuanxingUpOnly", TNumber},
  {"顶部底部均放置", "fk.Room_GuanxingBothSides", TNumber},
  {"只放置底部", "fk.Room_GuanxingDownOnly", TNumber},

  /* card move reasons */
  {"因使用而移动", "fk.ReasonUse", TNumber},
  {"因打出而移动", "fk.ReasonResponse", TNumber},
  {"因弃置而移动", "fk.ReasonDiscard", TNumber},
  // {"因重铸而移动", "fk.CardMoveReason_S_REASON_RECAST", TNumber},
  // {"因拼点而移动", "fk.CardMoveReason_S_REASON_PINDIAN", TNumber},
  {"因摸牌而移动", "fk.ReasonDraw", TNumber},
  {"因置入而移动", "fk.ReasonPut", TNumber},
  {"因交给而移动", "fk.ReasonGive", TNumber},
  {"因换牌而移动", "fk.ReasonExchange", TNumber},

  {NULL, NULL, TNone}
};

void fk_load_enum() {
  loadmodule(NULL, v);
}

char *fk_event_table[] = {
  "fk.NonTrigger",

  "fk.GameStart",
  "fk.TurnStart",
  "fk.EventPhaseStart",
  "fk.EventPhaseProceeding",
  "fk.EventPhaseEnd",
  "fk.EventPhaseChanging",
  "fk.EventPhaseSkipping",

  "fk.DrawNCards",
  "fk.AfterDrawNCards",
  "fk.DrawInitialCards",
  "fk.AfterDrawInitialCards",

  "fk.PreHpRecover",
  "fk.HpRecover",
  "fk.PreHpLost",
  "fk.HpLost",
  "fk.HpChanged",
  "fk.MaxHpChanged",

  "fk.EventLoseSkill",
  "fk.EventAcquireSkill",

  "fk.StartJudge",
  "fk.AskForRetrial",
  "fk.FinishRetrial",
  "fk.FinishJudge",

  "fk.PindianVerifying",
  "fk.Pindian",

  "fk.TurnedOver",
  "fk.ChainStateChanged",

  "fk.ConfirmDamage",
  "fk.Predamage",
  "fk.DamageForseen",
  "fk.DamageCaused",
  "fk.DamageInflicted",
  "fk.PreDamageDone",
  "fk.DamageDone",
  "fk.Damage",
  "fk.Damaged",
  "fk.DamageComplete",

  "fk.EnterDying",
  "fk.Dying",
  "fk.QuitDying",
  "fk.AskForPeaches",
  "fk.AskForPeachesDone",
  "fk.Death",
  "fk.BuryVictim",
  "fk.BeforeGameOverJudge",
  "fk.GameOverJudge",
  "fk.GameFinished",

  "fk.SlashEffected",
  "fk.SlashProceed",
  "fk.SlashHit",
  "fk.SlashMissed",

  "fk.JinkEffect",
  "fk.NullificationEffect",

  "fk.CardAsked",
  "fk.PreCardResponded",
  "fk.CardResponded",
  "fk.BeforeCardsMove",
  "fk.CardsMoveOneTime",

  "fk.PreCardUsed",
  "fk.CardUsed",
  "fk.TargetSpecifying",
  "fk.TargetConfirming",
  "fk.TargetSpecified",
  "fk.TargetConfirmed",
  "fk.CardEffect",
  "fk.CardEffected",
  "fk.PostCardEffected",
  "fk.CardFinished",
  "fk.TrickCardCanceling",
  "fk.TrickEffect",

  "fk.ChoiceMade",

  "fk.StageChange",
  "fk.FetchDrawPileCard",
  "fk.Debut",

  "fk.TurnBroken",

  "fk.NumOfEvents"
};
