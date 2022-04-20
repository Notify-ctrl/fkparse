#include "analyzer.h"
#include "enums.h"

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
  if (!s->skillspec->l) { /* empty spec */
    fprintf(yyout, "%ss%d = fkp.CreateTriggerSkill{\n  name = \"%ss%d\",\n}\n\n", readfile_name, currentskill->uid, readfile_name, currentskill->uid);
  } else {
    analyzeSkillspecs(s->skillspec);
  }
}

void analyzeSkillspecs(struct ast *a) {
  checktype(a->nodetype, N_SkillSpecs);

  if (a->l) {
    analyzeSkillspecs(a->l);
    struct ast *r = a->r;
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

  fprintf(yyout, "%ss%d = fkp.CreateTriggerSkill{\n  name = \"%ss%d\",\n  specs = {\n", readfile_name, currentskill->uid, readfile_name, currentskill->uid);
  indent_level++;
  analyzeTriggerspecs(a->l);
  fprintf(yyout, "  }\n}\n\n");
  indent_level--;
}

void analyzeTriggerspecs(struct ast *a) {
  checktype(a->nodetype, N_TriggerSpecs);

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
    case ConfirmDamage:
    case Predamage:
    case DamageForseen:
    case DamageCaused:
    case DamageInflicted:
      rewrite = 1;
    case PreDamageDone:
    case DamageDone:
    case Damage:
    case Damaged:
    case DamageComplete:
      break;
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
  writeline("[%d] = {", ts->event);
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
