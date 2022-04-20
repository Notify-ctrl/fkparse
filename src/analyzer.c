#include "analyzer.h"
#include "ast.h"
#include "linklist.h"
#include <stdlib.h>
#include <stdarg.h>

#define checktype(a, t) do {\
  if (a != t && a != TAny) { \
    fprintf(stderr, "Type error: expect %d, got %d\n", t, a); \
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

static void addgeneralskills(struct ast *skills) {
  struct ast *as = all_skills;
  if (skills->l) {
    addgeneralskills(skills->l);
    while (as->l) { // prev
      struct astskill *s = ((struct astskill *)(as->r));
      if (!strcmp(s->id->str, ((struct aststr *)(skills->r))->str)) {
        writeline("%sg%d:addSkill(%ss%d)", readfile_name, currentgeneral->uid, readfile_name, s->uid);
        return;
      }
      as = as->l;
    }
    fprintf(stderr, "不存在的技能 \"%s\"\n", ((struct aststr *)(skills->r))->str);
    exit(1);
  }
}

void analyzeGeneral(struct ast *a) {
  checktype(a->nodetype, N_General);

  struct astgeneral *g = (struct astgeneral *)a;
  currentgeneral = g;
  fprintf(yyout, "%sg%d = sgs.General(%sp%d, \"%sg%d\", \"%s\", %lld)\n",
         readfile_name, g->uid, readfile_name, currentpack->uid, readfile_name,
         g->uid, kingdom(g->kingdom->str), g->hp);
  char buf[64];
  sprintf(buf, "%sg%d", readfile_name, g->uid);
  addTranslation(buf, g->id->str);
  sprintf(buf, "#%sg%d", readfile_name, g->uid);
  addTranslation(buf, g->nickname->str);
  addgeneralskills(g->skills);
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

  writeline("require \"fkparser\"\n");
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
  if (!s->skillspec->l) { // empty spec
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

int analyzeExp(struct ast *);

int analyzeVar(struct ast *a) {
  checktype(a->nodetype, N_Var);

  int ret = TAny;
  struct astVar *v = (struct astVar *)a;

  if (v->obj) {
    // assuming obj is Player
    analyzeExp((struct ast *)(v->obj));
    char *s = v->name->str;
    if (!strcmp(s, "体力值")) {
      fprintf(yyout, ":getHp()");
      ret = TNumber;
    } else if (!strcmp(s, "手牌数")) {
      fprintf(yyout, ":getHandcardNum()");
      ret = TNumber;
    } else if (!strcmp(s, "体力上限")) {
      fprintf(yyout, ":getMaxHp()");
      ret = TNumber;
    } else {
      fprintf(stderr, "unknown field %s\n", v->name->str);
      exit(1);
    }
  } else {
    fprintf(yyout, "locals[\"%s\"]", v->name->str);
  }

  return ret;
}

// return the type of exp
int analyzeExp(struct ast *a) {
  checktype(a->nodetype, N_Exp);

  int ret = TNone;
  int t;

  struct astExp *e = (struct astExp *)a;
  if ((e->exptype == ExpCalc || e->exptype == ExpCmp) && e->optype != 0) {
    t = analyzeExp((struct ast *)(e->l));
    checktype(t, TNumber);
    switch (e->optype) {
      case 1: fprintf(yyout, " > "); break;
      case 2: fprintf(yyout, " < "); break;
      case 3: fprintf(yyout, " ~= "); break;
      case 4: fprintf(yyout, " == "); break;
      case 5: fprintf(yyout, " >= "); break;
      case 6: fprintf(yyout, " <= "); break;
      default: fprintf(yyout, " %c " ,e->optype); break;
    }
    t = analyzeExp((struct ast *)(e->r));
    checktype(t, TNumber);
    return TNumber;
  }

  switch (e->exptype) {
    case ExpNum: fprintf(yyout, "%lld" ,e->value); return TNumber;
    case ExpBool: fprintf(yyout, "%s" ,e->value == 0 ? "false" : "true"); return TNumber;
    case ExpStr: fprintf(yyout, "'%s'", ((struct aststr *)(e->l))->str); return TString;
    case ExpVar: return analyzeVar((struct ast *)(e->l));
    case ExpAction: return analyzeAction((struct ast *)(e->l));
    default: fprintf(stderr, "unknown exptype %d\n", e->exptype); exit(1);
  }

  return ret;
}

void analyzeBlock(struct ast *a) {
  checktype(a->nodetype, N_Block);

  analyzeStats(a->l);
}

void analyzeStats(struct ast *a) {
  checktype(a->nodetype, N_Stats);

  struct ast *stat;

  if (a->l) {
    analyzeStats(a->l);
    stat = a->r;
    switch (stat->nodetype) {
      case N_Stat_None: writeline(";"); break;
      case N_Stat_Assign: analyzeStatAssign(stat); break;
      case N_Stat_If: analyzeIf(stat); break;
      case N_Stat_Loop: analyzeLoop(stat); break;
      case N_Stat_Break: writeline("break"); break;
      case N_Stat_Ret: break;
      case N_Stat_Action: analyzeAction(stat); break;
      default: fprintf(stderr, "unexpected statement type %d\n", stat->nodetype); break;
    }
  }
}

void analyzeStatAssign(struct ast *a) {
  checktype(a->nodetype, N_Stat_Assign);

  print_indent();
  analyzeVar(a->l);
  fprintf(yyout, " = ");
  analyzeExp(a->r);
  fprintf(yyout, "\n");
}

void analyzeIf(struct ast *a) {
  checktype(a->nodetype, N_Stat_If);

  struct astIf *s = (struct astIf *)a;
  print_indent();
  fprintf(yyout, "if ");
  analyzeExp(s->cond);
  fprintf(yyout, " then\n");
  indent_level++;
  analyzeBlock(s->then);
  if (s->el) {
    indent_level--;
    writeline("else");
    indent_level++;
    analyzeBlock(s->el);
  }
  indent_level--;
  writeline("end");
}

void analyzeLoop(struct ast *a) {
  checktype(a->nodetype, N_Stat_Loop);

  struct astLoop *s = (struct astLoop *)a;
  writeline("repeat");
  indent_level++;
  analyzeBlock(s->body);
  indent_level--;
  print_indent();
  fprintf(yyout, "until ");
  analyzeExp(s->cond);
  fprintf(yyout, "\n");
}

int analyzeAction(struct ast *a) {
  checktype(a->nodetype, N_Stat_Action);

  int ret = TNone;
  int t;

  struct astAction *s = (struct astAction *)a;
  struct ast *action = s->action;
  switch (s->actiontype) {
    case ActionDrawcard:
      print_indent();
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ":drawCards(");
      t = analyzeExp(action->r);
      checktype(t, TNumber);
      fprintf(yyout, ", self:objectName())\n");
      break;
    case ActionLosehp:
      print_indent();
      fprintf(yyout, "room:loseHp(");
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r);
      checktype(t, TNumber);
      fprintf(yyout, ")\n");
      break;
    case ActionDamage:
      print_indent();
      fprintf(yyout, "room:damage(sgs.DamageStruct(self:objectName(), ");
      struct actionDamage *d = (struct actionDamage *)action;
      if (d->src) {
        t = analyzeExp(d->src);
        checktype(t, TPlayer);
      } else fprintf(yyout, "nil");
      fprintf(yyout, ", ");
      t = analyzeExp(d->dst); checktype(t, TPlayer); fprintf(yyout, ", ");
      t = analyzeExp(d->num); checktype(t, TNumber); fprintf(yyout, "))\n");
      break;
    case ActionRecover:
      print_indent();
      fprintf(yyout, "room:recover(");
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ", sgs.RecoverStruct(nil, nil, ");
      t = analyzeExp(action->r);
      checktype(t, TNumber);
      fprintf(yyout, "))\n");
      break;
    case ActionAcquireSkill:
      print_indent();
      fprintf(yyout, "room:acquireSkill(");
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r);
      checktype(t, TString);
      fprintf(yyout, ")\n");
      break;
    case ActionDetachSkill:
      print_indent();
      fprintf(yyout, "room:detachSkillFromPlayer(");
      t = analyzeExp(action->l);
      checktype(t, TPlayer);
      fprintf(yyout, ", ");
      t = analyzeExp(action->r);
      checktype(t, TString);
      fprintf(yyout, ")\n");
      break;
    default:
      fprintf(stderr, "unexpected action type %d\n", s->nodetype); break;
  }

  return ret;
}

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
