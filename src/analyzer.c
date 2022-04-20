#include "analyzer.h"
#include "ast.h"
#include "linklist.h"
#include <stdlib.h>

#define checktype(a, t) do {\
  if (a != t) { \
    yyerror("Type error: expect %d, got %d", t, a); \
    exit(1); \
  } \
} while(0)

static struct ast *all_skills;

static struct astpackage *currentpack;
static struct astgeneral *currentgeneral;
static struct astskill *currentskill;

static int indent_level = 0;

static void print_indent() {
  for (int i = 0; i < indent_level; i++)
    fprintf(yyout, "  ");
}

static void writeline(const char *msg, ...) {
  print_indent();
  va_list ap;
  va_start(ap, msg);

  vfprintf(yyout, msg, ap);
  fprintf(yyout, "\n");
}

static int exists(struct ast *skill, struct ast *str) {
  if (!strcmp(((struct astskill *)skill)->id->str, (char *)str)) {
    return 1;
  }
  return 0;
}

static char *kingdom(char *k) {
  if (!strcmp(k, "魏"))
    return "wei";
  else if (!strcmp(k, "蜀"))
    return "shu";
  else if (!strcmp(k, "吴"))
    return "wu";
  else if (!strcmp(k, "群"))
    return "qun";
  else if (!strcmp(k, "神"))
    return "god";
  else {
    fprintf(stderr, "未知国籍 \"%s\"：退出\n", k);
    exit(1);
  }
}

void analyzeGeneral(struct ast *a) {
  checktype(a->nodetype, N_General);

  struct astgeneral *g = (struct astgeneral *)a;
  fprintf(yyout, "%sg%d = sgs.General(%sp%d, \"%sg%d\", \"%s\", %lld)\n",
         readfile_name, g->uid, readfile_name, currentpack->uid, readfile_name,
         g->uid, kingdom(g->kingdom->str), g->hp);
  char buf[64];
  sprintf(buf, "%sg%d", readfile_name, g->uid);
  addTranslation(buf, g->id->str);
  sprintf(buf, "#%sg%d", readfile_name, g->uid);
  addTranslation(buf, g->nickname->str);
}

void analyzePackageList(struct ast *a) {
  checktype(a->nodetype, N_Packages);

  if (a->l) {
    analyzePackageList(a->l);
  }

  analyzePackage(a->r);
}

void analyzePackage(struct ast *a) {
  checktype(a->nodetype, N_Package);

  struct astpackage *p = (struct astpackage *)a;
  currentpack = p;
  fprintf(yyout, "%sp%d = sgs.Package(\"%sp%d\")\n",readfile_name, p->uid, readfile_name, p->uid);
  char buf[64];
  sprintf(buf, "%sp%d", readfile_name, p->uid);
  addTranslation(buf, ((struct aststr *)p->id)->str);
  analyzeGeneralList(a->r);
  fprintf(yyout, "\n");
}

void analyzeExtension(struct ast *a) {
  checktype(a->nodetype, N_Extension);
  all_skills = a->l;
  analyzeSkillList(a->l);
  analyzePackageList(a->r);
  loadTranslations();
  fprintf(yyout, "\nreturn { ");
  for (int i = 0; i <= currentpack->uid; i++) {
    fprintf(yyout, "%sp%d, ", readfile_name, i);
  }
  fprintf(yyout, "}\n");
}

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
  analyzeSkillspecs(s->skillspec);
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

void analyzeBlock(struct ast *a) {
  checktype(a->nodetype, N_Block);
}

void analyzeStats(struct ast *a) {}
void analyzeStatAssign(struct ast *a) {}
void analyzeIf(struct ast *a) {}
void analyzeLoop(struct ast *a) {}
void analyzeAction(struct ast *a) {}



void analyzeGeneralList(struct ast *a) {
  checktype(a->nodetype, N_Generals);

  if (a->l) {
    analyzeGeneralList(a->l);
    analyzeGeneral(a->r);
  }
}



void analyzeStringList(struct ast *a) {
  checktype(a->nodetype, N_Strs);

  if (a->l) {
    analyzeStringList(a->l);
    analyzeString(a->r);
  }
}

void analyzeString(struct ast *a) {
  checktype(a->nodetype, N_Str);

  fprintf(yyout, "\"%s\",", ((struct aststr *)a)->str);
}
