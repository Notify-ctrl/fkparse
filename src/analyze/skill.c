#include "analyzer.h"
#include "enums.h"

/* for skill spec analyzing */
static int analyzedSpecs[16];
static int specStackPointer;
static int hasTriggerSkill;
static int hasViewAsSkill;

/* trigger skill analyzing */
static int analyzedEvents[256];
static int eventStackPointer;

void analyzeSkillList(struct ast *a) {
  checktype(a->nodetype, N_Skills);

  if (a->l) {
    analyzeSkillList(a->l);
    analyzeSkill(a->r);
  }
}

void analyzeSkill(struct ast *a) {
  checktype(a->nodetype, N_Skill);

  struct astskill *s = (struct astskill *)a;
  currentskill = s;

  char buf[64];
  sprintf(buf, "%s", s->interid->str);
  addTranslation(buf, s->id->str);
  sprintf(buf, ":%s", s->interid->str);
  addTranslation(buf, s->description->str);

  hasTriggerSkill = 0;
  hasViewAsSkill = 0;

  /* pre-analyze specs */
  struct ast *specs = s->skillspec;
  while (specs->r) {
    if (specs->r->nodetype == N_TriggerSkill)
      hasTriggerSkill = 1;
    specs = specs->l;
  }

  analyzeSkillspecs(s->skillspec);

  if (!hasTriggerSkill) { /* no trigger skill, create a dummy one */
    fprintf(yyout, "%s = fkp.CreateTriggerSkill{\n  name = \"%s\",\n  frequency = ", s->interid->str, s->interid->str);
    analyzeReserved(s->frequency->str);
    fprintf(yyout, ",\n}\n\n");
  }
}

static void checkDuplicate(int *arr, int to_check, int p, char *msg) {
  for (int i = 0; i < p; i++) {
    if (arr[i] == to_check) {
      fprintf(stderr, "错误：%s %d\n", msg, to_check);
      exit(1);
    }
  }
}

void analyzeSkillspecs(struct ast *a) {
  checktype(a->nodetype, N_SkillSpecs);

  memset(analyzedSpecs, 0, sizeof(int) * 16);
  specStackPointer = 0;

  if (a->l) {
    analyzeSkillspecs(a->l);

    struct ast *r = a->r;
    checkDuplicate(analyzedSpecs, r->nodetype, specStackPointer, "重复的技能类型");
    analyzedSpecs[specStackPointer] = r->nodetype;
    specStackPointer++;
    switch (r->nodetype) {
      case N_TriggerSkill:
        analyzeTriggerSkill(r);
        break;

      default:
        yyerror("Unexpected Skill type %d\n", r->nodetype);
        exit(1);
    }
  }
}

void analyzeTriggerSkill(struct ast *a) {
  checktype(a->nodetype, N_TriggerSkill);

  fprintf(yyout, "%s = fkp.CreateTriggerSkill{\n  name = \"%s\",\n  frequency = ", currentskill->interid->str, currentskill->interid->str);
  analyzeReserved(currentskill->frequency->str);
  fprintf(yyout, ",\n  specs = {\n");
  indent_level++;
  analyzeTriggerspecs(a->l);
  fprintf(yyout, "  }\n}\n\n");
  indent_level--;
}

void analyzeTriggerspecs(struct ast *a) {
  checktype(a->nodetype, N_TriggerSpecs);

  memset(analyzedEvents, 0, sizeof(int) * 256);
  eventStackPointer = 0;

  if (a->l) {
    analyzeTriggerspecs(a->l);
  }

  analyzeTriggerspec(a->r);
}

static void defineLocal(char *k, char *v, int type) {
  writeline("locals[\"%s\"] = %s", k, v);
  lookup(k)->type = type;
}

static void initData(int event) {
  writeline("-- get datas for this trigger event");
  switch (event) {
    case DrawNCards:
    case AfterDrawNCards:
    case DrawInitialCards:
    case AfterDrawInitialCards:
      defineLocal("摸牌数量", "data:toInt()", TNumber);
      break;

    case PreHpRecover:
    case HpRecover:
      writeline("local recover = data:toRecover()");
      defineLocal("回复来源", "recover.who", TPlayer);
      defineLocal("回复的牌", "recover.card", TCard);
      defineLocal("回复值", "recover.recover", TNumber);
      break;
      
    case PreHpLost:
    case HpLost:
      defineLocal("失去值", "data:toInt()", TNumber);
      break;

    case EventLoseSkill:
    case EventAcquireSkill:
      defineLocal("技能", "data:toString()", TString);
      break;

    case StartJudge:
    case AskForRetrial:
    case FinishRetrial:
    case FinishJudge:
      writeline("local judge = data:toJudge()");
      defineLocal("判定效果是负收益", "judge.negative", TBool);
      defineLocal("播放判定动画", "judge.play_animation", TBool);
      defineLocal("判定角色", "judge.who", TPlayer);
      defineLocal("判定牌", "judge.card", TCard);
      defineLocal("判定类型", "judge.pattern", TNumber);
      defineLocal("判定结果", "judge.good", TBool);
      defineLocal("判定原因", "judge.reason", TString);
      defineLocal("判定时间延迟", "judge.time_consuming", TBool);
      defineLocal("可改判角色", "judge.retrial_by_response", TPlayer);
      break;

    case PindianVerifying:
    case Pindian:
      writeline("local pindian = data:toPindian()");
      defineLocal("拼点来源", "pindian.from", TPlayer);
      defineLocal("拼点目标", "pindian.to", TPlayer);
      defineLocal("拼点来源的牌", "pindian.from_card", TCard);
      defineLocal("拼点目标的牌", "pindian.to_card", TCard);
      defineLocal("拼点来源的点数", "pindian.from_number", TNumber);
      defineLocal("拼点目标的点数", "pindian.to_number", TNumber);
      defineLocal("拼点原因", "pindian.reason", TString);
      defineLocal("拼点成功", "pindian.success", TBool);
      break;

    case ConfirmDamage:
    case Predamage:
    case DamageForseen:
    case DamageCaused:
    case DamageInflicted:
    case PreDamageDone:
    case DamageDone:
    case Damage:
    case Damaged:
    case DamageComplete:
      writeline("local damage = data:toDamage()");
      defineLocal("伤害来源", "damage.from", TPlayer);
      defineLocal("伤害目标", "damage.to", TPlayer);
      defineLocal("造成伤害的牌", "damage.card", TCard);
      defineLocal("伤害值", "damage.damage", TNumber);
      defineLocal("伤害属性", "damage.nature", TNumber);
      defineLocal("伤害是传导伤害", "damage.chain", TBool);
      defineLocal("伤害是转移伤害", "damage.transfer", TBool);
      defineLocal("是目标角色", "damage.by_user", TBool);
      defineLocal("造成伤害的原因", "damage.reason", TString);
      defineLocal("伤害传导的原因", "damage.transfer_reason", TString);
      defineLocal("伤害被防止", "damage.prevented", TBool);
      break;

    case EventPhaseChanging:
      writeline("local change = data:toPhaseChange()");
      defineLocal("上个阶段", "change.from", TNumber);
      defineLocal("下个阶段", "change.to", TNumber);
      break;

    case EnterDying:
    case Dying:
    case QuitDying:
    case AskForPeaches:
    case AskForPeachesDone:
      writeline("local dying = data:toDying()");
      defineLocal("濒死的角色", "dying.who", TPlayer);
      defineLocal("濒死的伤害来源", "dying.damage.from", TPlayer);
      defineLocal("濒死的伤害目标", "dying.damage.to", TPlayer);
      defineLocal("造成濒死的伤害的牌", "dying.damage.card", TCard);
      defineLocal("濒死的伤害值", "dying.damage.damage", TNumber);
      defineLocal("濒死的伤害属性", "dying.damage.nature", TNumber);
      defineLocal("濒死的伤害是传导伤害", "dying.damage.chain", TBool);
      defineLocal("濒死的伤害是转移伤害", "dying.damage.transfer", TBool);
      defineLocal("濒死的角色是目标角色", "dying.damage.by_user", TBool);
      defineLocal("造成濒死的伤害的原因", "dying.damage.reason", TString);
      defineLocal("濒死的伤害传导的原因", "dying.damage.transfer_reason", TString);
      defineLocal("濒死的伤害被防止", "dying.damage.prevented", TBool);
      break;

    case Death:
    case BuryVictim:
    case BeforeGameOverJudge:
    case GameOverJudge:
      writeline("local death = data:toDeath()");
      defineLocal("死亡的角色", "death.who", TPlayer);
      defineLocal("死亡的伤害来源", "death.damage.from", TPlayer);
      defineLocal("死亡的伤害目标", "death.damage.to", TPlayer);
      defineLocal("造成死亡的伤害的牌", "death.damage.card", TCard);
      defineLocal("死亡的伤害值", "death.damage.damage", TNumber);
      defineLocal("死亡的伤害属性", "death.damage.nature", TNumber);
      defineLocal("死亡的伤害是传导伤害", "death.damage.chain", TBool);
      defineLocal("死亡的伤害是转移伤害", "death.damage.transfer", TBool);
      defineLocal("死亡的角色是目标角色", "death.damage.by_user", TBool);
      defineLocal("造成死亡的伤害的原因", "death.damage.reason", TString);
      defineLocal("死亡的伤害传导的原因", "death.damage.transfer_reason", TString);
      defineLocal("死亡的伤害被防止", "death.damage.prevented", TBool);
      break;

    case PreCardResponded:
    case CardResponded:
      writeline("local resp = data:toCardResponse()");
      defineLocal("响应的牌", "resp.m_card", TCard);
      defineLocal("响应的目标", "resp.m_who", TPlayer);
      defineLocal("响应的牌被使用", "resp.m_isUse", TBool);
      defineLocal("响应的牌被改判", "resp.m_isRetrial", TBool);
      defineLocal("响应的牌是手牌", "resp.m_isHandcard", TBool);
      break;

    case BeforeCardsMove:
    case CardsMoveOneTime:
      writeline("local move = data:toMoveOneTime()");
      defineLocal("移动的牌", "move.card_ids", TNumber);
      defineLocal("移动产地", "move.from_place", TNumber);
      defineLocal("移动目的地", "move.to_place", TNumber);
      defineLocal("移动的来源的名字", "move.from_player_name", TString);
      defineLocal("移动的目标的名字", "move.to_player_name", TString);
      defineLocal("移动的牌堆", "move.from_pile_name", TString);
      defineLocal("目的地牌堆", "move.to_pile_name", TString);
      defineLocal("移动的来源", "move.from", TPlayer);
      defineLocal("移动的目标", "move.to", TPlayer);
      defineLocal("移动的原因", "move.reason", TString);
      defineLocal("是打开的", "move.open", TBool);
      defineLocal("是最后的手牌", "move.is_last_handcard", TBool);
      break;

    case CardUsed:
    case TargetSpecifying:
    case TargetConfirming:
    case TargetSpecified:
    case TargetConfirmed:
    case CardFinished:
      writeline("local use = data:toCardUse()");
      defineLocal("使用的牌", "use.card", TCard);
      defineLocal("使用者", "use.from", TPlayer);
      defineLocal("目标", "use.to", TPlayer);
      defineLocal("持有者是使用者", "use.m_isOwnerUse", TBool);
      defineLocal("计入使用次数", "use.m_addHistory", TBool);
      defineLocal("是手牌", "use.m_isHandcard", TBool);
      defineLocal("无效的目标", "use.nullified_list", TNumber);
      break;

    case CardEffected:
      writeline("local effect = data:toCardEffect()");
      defineLocal("生效的牌", "effect.card", TCard);
      defineLocal("使用者", "effect.from", TPlayer);
      defineLocal("目标", "effect.to", TPlayer);
      defineLocal("是否生效", "effect.nullified", TBool);
      break;

    default:
      break;
  }
  writeline("");
}

static void clearLocal(char *k, char *v, int rewrite) {
  lookup(k)->type = TNone;
  if (rewrite) {
    writeline("%s = locals[\"%s\"]", v, k);
  }
}

static void clearData(int event) {
  int rewrite = 0;
  switch (event) {
    case DrawNCards:
    case DrawInitialCards:
    case PreHpRecover:
    case PreHpLost:
    case PindianVerifying:
    case PreCardResponded:
    case BeforeCardsMove:
    case TargetSpecifying:
    case TargetConfirming:
    case ConfirmDamage:
    case Predamage:
    case DamageForseen:
    case DamageCaused:
    case DamageInflicted:
      rewrite = 1;

    default:
      break;
  }

  switch (event) {
    case DrawNCards:
    case AfterDrawNCards:
    case DrawInitialCards:
    case AfterDrawInitialCards:
      clearLocal("摸牌数量", "data:toInt()", rewrite);
      if (rewrite) writeline("data:setValue(data:toInt())");
      break;

    case PreHpRecover:
    case HpRecover:
      if (rewrite) writeline("");
      clearLocal("回复来源", "recover.who", rewrite);
      clearLocal("回复的牌", "recover.card", rewrite);
      clearLocal("回复值", "recover.recover", rewrite);
      if (rewrite) writeline("data:setValue(recover)");
      break;
      
    case PreHpLost:
    case HpLost:
      clearLocal("失去值", "data:toInt()", rewrite);
      if (rewrite) writeline("data:setValue(data:toInt())");
      break;

    case EventLoseSkill:
    case EventAcquireSkill:
      clearLocal("技能", "data:toString()", rewrite);
      if (rewrite) writeline("data:setValue(data:toString())");
      break;

    case StartJudge:
    case AskForRetrial:
    case FinishRetrial:
    case FinishJudge:
      if (rewrite) writeline("");
      clearLocal("判定效果是负收益", "judge.negative", rewrite);
      clearLocal("播放判定动画", "judge.play_animation", rewrite);
      clearLocal("判定角色", "judge.who", rewrite);
      clearLocal("判定牌", "judge.card", rewrite);
      clearLocal("判定类型", "judge.pattern", rewrite);
      clearLocal("判定结果", "judge.good", rewrite);
      clearLocal("判定原因", "judge.reason", rewrite);
      clearLocal("判定时间延迟", "judge.time_consuming", rewrite);
      clearLocal("可改判角色", "judge.retrial_by_response", rewrite);
      if (rewrite) writeline("data:setValue(judge)");
      break;

    case PindianVerifying:
    case Pindian:
      if (rewrite) writeline("");
      clearLocal("拼点来源", "pindian.from", rewrite);
      clearLocal("拼点目标", "pindian.to", rewrite);
      clearLocal("拼点来源的牌", "pindian.from_card", rewrite);
      clearLocal("拼点目标的牌", "pindian.to_card", rewrite);
      clearLocal("拼点来源的点数", "pindian.from_number", rewrite);
      clearLocal("拼点目标的点数", "pindian.to_number", rewrite);
      clearLocal("拼点原因", "pindian.reason", rewrite);
      clearLocal("拼点成功", "pindian.success", rewrite);
      if (rewrite) writeline("data:setValue(pindian)");
      break;

    case ConfirmDamage:
    case Predamage:
    case DamageForseen:
    case DamageCaused:
    case DamageInflicted:
    case PreDamageDone:
    case DamageDone:
    case Damage:
    case Damaged:
    case DamageComplete:
      if (rewrite) writeline("");
      clearLocal("伤害来源", "damage.from", rewrite);
      clearLocal("伤害目标", "damage.to", rewrite);
      clearLocal("造成伤害的牌", "damage.card", rewrite);
      clearLocal("伤害值", "damage.damage", rewrite);
      clearLocal("伤害属性", "damage.nature", rewrite);
      clearLocal("伤害是传导伤害", "damage.chain", rewrite);
      clearLocal("伤害是转移伤害", "damage.transfer", rewrite);
      clearLocal("是目标角色", "damage.by_user", rewrite);
      clearLocal("造成伤害的原因", "damage.reason", rewrite);
      clearLocal("伤害传导的原因", "damage.transfer_reason", rewrite);
      clearLocal("伤害被防止", "damage.prevented", rewrite);
      if (rewrite) writeline("data:setValue(damage)");
      break;

    case EventPhaseChanging:
      if (rewrite) writeline("");
      clearLocal("上个阶段", "change.from", rewrite);
      clearLocal("下个阶段", "change.to", rewrite);
      if (rewrite) writeline("data:setValue(change)");
      break;

    case EnterDying:
    case Dying:
    case QuitDying:
    case AskForPeaches:
    case AskForPeachesDone:
      if (rewrite) writeline("");
      clearLocal("濒死的角色", "dying.who", rewrite);
      clearLocal("濒死的伤害来源", "dying.damage.from", rewrite);
      clearLocal("濒死的伤害目标", "dying.damage.to", rewrite);
      clearLocal("造成濒死的伤害的牌", "dying.damage.card", rewrite);
      clearLocal("濒死的伤害值", "dying.damage.damage", rewrite);
      clearLocal("濒死的伤害属性", "dying.damage.nature", rewrite);
      clearLocal("濒死的伤害是传导伤害", "dying.damage.chain", rewrite);
      clearLocal("濒死的伤害是转移伤害", "dying.damage.transfer", rewrite);
      clearLocal("濒死的角色是目标角色", "dying.damage.by_user", rewrite);
      clearLocal("造成濒死的伤害的原因", "dying.damage.reason", rewrite);
      clearLocal("濒死的伤害传导的原因", "dying.damage.transfer_reason", rewrite);
      clearLocal("濒死的伤害被防止", "dying.damage.prevented", rewrite);
      if (rewrite) writeline("data:setValue(dying)");
      break;

    case Death:
    case BuryVictim:
    case BeforeGameOverJudge:
    case GameOverJudge:
      if (rewrite) writeline("");
      clearLocal("死亡的角色", "death.who", rewrite);
      clearLocal("死亡的伤害来源", "death.damage.from", rewrite);
      clearLocal("死亡的伤害目标", "death.damage.to", rewrite);
      clearLocal("造成死亡的伤害的牌", "death.damage.card", rewrite);
      clearLocal("死亡的伤害值", "death.damage.damage", rewrite);
      clearLocal("死亡的伤害属性", "death.damage.nature", rewrite);
      clearLocal("死亡的伤害是传导伤害", "death.damage.chain", rewrite);
      clearLocal("死亡的伤害是转移伤害", "death.damage.transfer", rewrite);
      clearLocal("死亡的角色是目标角色", "death.damage.by_user", rewrite);
      clearLocal("造成死亡的伤害的原因", "death.damage.reason", rewrite);
      clearLocal("死亡的伤害传导的原因", "death.damage.transfer_reason", rewrite);
      clearLocal("死亡的伤害被防止", "death.damage.prevented", rewrite);
      if (rewrite) writeline("data:setValue(death)");
      break;

    case PreCardResponded:
    case CardResponded:
      if (rewrite) writeline("");
      clearLocal("响应的牌", "resp.m_card", rewrite);
      clearLocal("响应的目标", "resp.m_who", rewrite);
      clearLocal("响应的牌被使用", "resp.m_isUse", rewrite);
      clearLocal("响应的牌被改判", "resp.m_isRetrial", rewrite);
      clearLocal("响应的牌是手牌", "resp.m_isHandcard", rewrite);
      if (rewrite) writeline("data:setValue(resp)");
      break;

    case BeforeCardsMove:
    case CardsMoveOneTime:
      if (rewrite) writeline("");
      clearLocal("移动的牌", "move.card_ids", rewrite);
      clearLocal("移动产地", "move.from_place", rewrite);
      clearLocal("移动目的地", "move.to_place", rewrite);
      clearLocal("移动的来源的名字", "move.from_player_name", rewrite);
      clearLocal("移动的目标的名字", "move.to_player_name", rewrite);
      clearLocal("移动的牌堆", "move.from_pile_name", rewrite);
      clearLocal("目的地牌堆", "move.to_pile_name", rewrite);
      clearLocal("移动的来源", "move.from", rewrite);
      clearLocal("移动的目标", "move.to", rewrite);
      clearLocal("移动的原因", "move.reason", rewrite);
      clearLocal("是打开的", "move.open", rewrite);
      clearLocal("是最后的手牌", "move.is_last_handcard", rewrite);
      if (rewrite) writeline("data:setValue(move)");
      break;

    case CardUsed:
    case TargetSpecifying:
    case TargetConfirming:
    case TargetSpecified:
    case TargetConfirmed:
    case CardFinished:
      if (rewrite) writeline("");
      clearLocal("使用的牌", "use.card", rewrite);
      clearLocal("使用者", "use.from", rewrite);
      clearLocal("目标", "use.to", rewrite);
      clearLocal("持有者是使用者", "use.m_isOwnerUse", rewrite);
      clearLocal("计入使用次数", "use.m_addHistory", rewrite);
      clearLocal("是手牌", "use.m_isHandcard", rewrite);
      clearLocal("无效的目标", "use.nullified_list", rewrite);
      if (rewrite) writeline("data:setValue(use)");
      break;

    case CardEffected:
      if (rewrite) writeline("");
      clearLocal("生效的牌", "effect.card", rewrite);
      clearLocal("使用者", "effect.from", rewrite);
      clearLocal("目标", "effect.to", rewrite);
      clearLocal("是否生效", "effect.nullified", rewrite);
      if (rewrite) writeline("data:setValue(effect)");
      break;

    default:
      break;
  }
}

void analyzeTriggerspec(struct ast *a) {
  checktype(a->nodetype, N_TriggerSpec);

  struct astTriggerSpec *ts = (struct astTriggerSpec *)a;
  checkDuplicate(analyzedEvents, ts->event, eventStackPointer, "重复的时机");
  analyzedEvents[eventStackPointer] = ts->event;
  eventStackPointer++;

  writeline("[%s] = {", event_table[ts->event]);
  indent_level++;
  writeline("-- can_trigger");
  writeline("function (self, target, player, data)");
  indent_level++;
  if (ts->cond) {
    writeline("local room = player:getRoom()");
    writeline("local locals = {}\n");
    initData(ts->event);
    analyzeBlock(ts->cond);
    clearData(ts->event);
  } else {
    writeline("return target and player == target and player:hasSkill(self:objectName())");
  }

  indent_level--;
  writeline("end,\n");

  writeline("-- on effect");
  writeline("function (self, target, player, data)");
  indent_level++;
  writeline("local room = player:getRoom()");
  writeline("local locals = {}\n");
  initData(ts->event);
  analyzeBlock(ts->effect);
  clearData(ts->event);
  indent_level--;
  writeline("end,");

  indent_level--;
  writeline("},");
}
