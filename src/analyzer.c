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

static int skill_exists;

static int exists(struct ast *skill, struct ast *str) {
  if (!strcmp(((struct astskill *)skill)->id->str, (char *)str)) {
    return 1;
  }
  return 0;
}

static int general_addSkill(struct ast *skill, struct ast *general) {
  struct astgeneral *g = (struct astgeneral *)general;
  if (foreach(all_skills, (struct ast *)(((struct aststr *)skill)->str), exists))
    printf("%s:addSkill(\"%s\")\n", g->id->str, ((struct aststr *)skill)->str);
  else
    yyerror("%s not exist\n", ((struct aststr *)skill)->str);
  return 1;
}

void analyzeGeneral(struct ast *a) {
  checktype(a->nodetype, N_General);

  struct astgeneral *g = (struct astgeneral *)a;
  printf("%s,%s,%s,%lld\n", g->kingdom->str, g->nickname->str, g->id->str, g->hp);
  foreach(g->skills, (struct ast *)g, general_addSkill);
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

  printf("Package: %s\nGenerals:\n", ((struct aststr *)(a->l))->str);
  analyzeGeneralList(a->r);
  printf("\n");
}

void analyzeExtension(struct ast *a) {
  checktype(a->nodetype, N_Extension);
  all_skills = a->l;
  analyzeSkillList(a->l);
  analyzePackageList(a->r);
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
  printf("Read Skill: %s\n  description: %s\n", s->id->str, s->description->str);
  analyzeSkillspecs(s->skillspec);
  printf("\n");
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

  printf("  specs: [\n");
  analyzeTriggerspecs(a->l);
  printf("  ]\n");
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
  printf("Event: %d\n", ts->event);
  if (ts->cond)
    analyzeBlock(ts->cond);

  analyzeBlock(ts->effect);
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

  printf("\"%s\",", ((struct aststr *)a)->str);
}
