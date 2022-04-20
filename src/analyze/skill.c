#include "analyzer.h"

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

void analyzeTriggerspec(struct ast *a) {
  checktype(a->nodetype, N_TriggerSpec);

  struct astTriggerSpec *ts = (struct astTriggerSpec *)a;
  writeline("[%d] = {", ts->event);
  indent_level++;
  writeline("-- can_trigger");
  writeline("function (self, target, player, data)");
  indent_level++;
  if (ts->cond) {
    analyzeBlock(ts->cond);
  } else {
    writeline("return target and player == target and player:hasSkill(self:objectName())");
  }

  indent_level--;
  writeline("end,\n");

  writeline("-- on effect");
  writeline("function (self, target, player, data)");
  indent_level++;
  writeline("local room = player:getRoom()");
  writeline("local locals = {}");
  writeline("locals[\"你\"] = player\n");
  analyzeBlock(ts->effect);
  indent_level--;
  writeline("end,");

  indent_level--;
  writeline("},");
}
