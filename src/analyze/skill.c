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
  sprintf(buf, "%ss%d", readfile_name, s->uid);
  addTranslation(buf, s->id->str);
  sprintf(buf, ":%ss%d", readfile_name, s->uid);
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
    fprintf(yyout, "%ss%d = fkp.CreateTriggerSkill{\n  name = \"%ss%d\",\n  frequency = ", readfile_name, s->uid, readfile_name, s->uid);
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

  fprintf(yyout, "%ss%d = fkp.CreateTriggerSkill{\n  name = \"%ss%d\",\n  frequency = ", readfile_name, currentskill->uid, readfile_name, currentskill->uid);
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
      break;
    case PreHpLost:
    case HpLost:
      break;

    case EventLoseSkill:
    case EventAcquireSkill:
      break;

    case StartJudge:
    case AskForRetrial:
    case FinishRetrial:
    case FinishJudge:
      break;

    case PindianVerifying:
    case Pindian:
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
      break;

    case EnterDying:
    case Dying:
    case QuitDying:
    case AskForPeaches:
    case AskForPeachesDone:
      break;

    case Death:
    case BuryVictim:
    case BeforeGameOverJudge:
    case GameOverJudge:
      break;

    case PreCardResponded:
    case CardResponded:
      break;

    case BeforeCardsMove:
    case CardsMoveOneTime:
      break;

    case CardUsed:
    case TargetSpecifying:
    case TargetConfirming:
    case TargetSpecified:
    case TargetConfirmed:
    case CardFinished:
      break;

    case CardEffected:
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
      if (rewrite) writeline("data:setValue(damage)");
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
