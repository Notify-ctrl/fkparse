#include "ast.h"
#include "main.h"

struct ast *newast(NodeType nodetype, struct ast *l, struct ast *r) {
  struct ast *a = malloc(sizeof(struct ast));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *newextension(struct ast *funcList, struct ast *skillList,
                         struct ast *pkgList)
{
  struct astextension *a = malloc(sizeof(struct astextension));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_Extension;
  a->funcList = funcList;
  a->skillList = skillList;
  a->pkgList = pkgList;

  return (struct ast *)a;
}

struct ast *newnum(long long n) {
  struct numval *a = malloc(sizeof(struct numval));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_Num;
  a->n = n;
  return (struct ast *)a;
}

struct ast *newstr(char *s) {
  struct aststr *a = malloc(sizeof(struct aststr));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_Str;
  a->str = strdup(s);
  return (struct ast *)a;
}

struct ast *newfuncdef(struct ast *name, struct ast *params,
                              struct ast *funcbody)
{
  struct astfuncdef *a = malloc(sizeof(struct astfuncdef));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_Funcdef;
  a->name = (struct aststr *)name;
  a->params = params;
  a->funcbody = funcbody;

  return (struct ast *)a;
}

struct ast *newpackage(char *id, struct ast *generals) {
  struct astpackage *a = malloc(sizeof(struct astpackage));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_Package;
  a->id = newstr(id);
  a->generals = generals;
  static int package_id = 0;
  a->uid = package_id++;
  return (struct ast *)a;
}

struct ast *newgeneral(char *id, char *kingdom, long long hp,
                        char *nickname, char *gender, char *interid, struct ast *skills) {
  struct astgeneral *a = malloc(sizeof(struct astgeneral));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_General;
  a->id = (struct aststr *)newstr(id);
  a->kingdom = (struct aststr *)newstr(kingdom);
  a->hp = hp;
  a->nickname = (struct aststr *)newstr(nickname);
  a->gender = gender ? (struct aststr *)newstr(gender)
                     : (struct aststr *)newstr("男性");
  a->skills = skills;
  static int general_id = 0;
  a->uid = general_id++;
  char buf[64];
  sprintf(buf, "%sg%d", readfile_name, a->uid);
  a->interid = interid ? (struct aststr *)newstr(interid)
                       : (struct aststr *)newstr(buf);
  return (struct ast *)a;
}

struct ast *newskill(char *id, char *desc, char *frequency, char *interid, struct ast *spec) {
  struct astskill *a = malloc(sizeof(struct astskill));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_Skill;
  a->id = (struct aststr *)newstr(id);
  a->description = (struct aststr *)newstr(desc);
  a->frequency = frequency ? (struct aststr *)newstr(frequency)
                           : (struct aststr *)newstr("普通技");
  a->skillspecs = spec;
  static int skill_id = 0;
  a->uid = skill_id++;
  char buf[64];
  sprintf(buf, "%ss%d", readfile_name, a->uid);
  a->interid = interid ? (struct aststr *)newstr(interid)
                       : (struct aststr *)newstr(buf);
  return (struct ast *)a;
}

struct ast *newtriggerspec(int event, struct ast *cond, struct ast *effect) {
  struct astTriggerSpec *a = malloc(sizeof(struct astTriggerSpec));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_TriggerSpec;
  a->event = event;
  a->cond = cond;
  a->effect = effect;
  return (struct ast *)a;
}

struct ast *newif(struct ast *cond, struct ast *then, struct ast *el) {
  struct astIf *a = malloc(sizeof(struct astIf));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_Stat_If;
  a->cond = cond;
  a->then = then;
  a->el = el;
  return (struct ast *)a;
}

struct ast *newaction(int type, struct ast *action) {
  struct astAction *a = malloc(sizeof(struct astAction));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_Action;
  a->actiontype = type;
  a->action = action;
  a->standalone = 1;
  return (struct ast *)a;
}

struct ast *newdamage(struct ast *src, struct ast *dst, struct ast *num) {
  struct actionDamage *a = malloc(sizeof(struct actionDamage));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = -1;
  a->src = src;
  a->dst = dst;
  a->num = num;
  return (struct ast *)a;
}

struct ast *newmark(struct ast *player, char *name, struct ast *num, int hidden, int optype) {
  struct actionMark *a = malloc(sizeof(struct actionMark));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = -1;
  a->player = player;
  a->name = (struct aststr *)newstr(name);
  a->num = num;
  a->hidden = hidden;
  a->optype = optype;
  return (struct ast *)a;
}

struct ast *newexp(int exptype, long long value, int optype, struct astExp *l, struct astExp *r) {
  struct astExp *a = malloc(sizeof(struct astExp));

  if(!a) {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = N_Exp;
  a->exptype = exptype;
  switch (exptype) {
    case ExpCmp:
    case ExpCalc:
    case ExpNum:
      a->valuetype = TNumber;
      break;
    case ExpStr:
      a->valuetype = TString;
      break;
    default:
      a->valuetype = -1;
  }
  a->value = value;
  a->optype = optype;
  a->l = l;
  a->r = r;
  a->bracketed = 0;
  return (struct ast *)a;
}
